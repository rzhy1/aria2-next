# AGENTS.md - aria2-next

This file provides operating instructions for AI coding agents working in this repository. Human contributors should start with [README.md](README.md), [packaging/README.md](packaging/README.md), [tools/README.md](tools/README.md), [third_party/README.md](third_party/README.md), and [maintenance/README.md](maintenance/README.md).

> [!IMPORTANT]
> All changes must meet industrial-grade quality. Find the root cause before changing code, preserve existing behavior unless the task explicitly requires a behavior change, keep edits narrowly scoped, and verify the exact build or release path affected by the change.

## Project Architecture

| Area | Stack / Ownership |
| --- | --- |
| Core | C99 and C++11 aria2 command-line client and core implementation |
| Build | CMake 3.25+ with Ninja as the default generator |
| Tests | CTest plus the existing CppUnit-based test suite |
| Packaging | Cross-platform release automation under `packaging/` and `.github/workflows/release.yml` |
| Third-party source | Vendored `third_party/wslay` with upstream layout and style preserved |

### Key Paths

```text
CMakeLists.txt                  Root CMake build definition and project version source
CMakePresets.json               Standard local configure, build, and test presets
cmake/                          CMake templates, generated config headers, source inventories
src/                            aria2 core, CLI, protocol, disk, RPC, and platform code
src/includes/aria2/             Public libaria2 headers
test/                           CppUnit tests registered through CTest
doc/                            Manual, manpage, completion, and documentation tooling
packaging/                      Release dependencies, Dockerfiles, cross-build scripts, package assets
packaging/dependencies.env      Authoritative release dependency baseline
third_party/                    Bundled third-party source with explicit ownership rules
tools/                          General repository helper scripts
maintenance/                    Durable maintenance records, not build inputs
.github/workflows/release.yml   Six-platform release build and draft GitHub Release workflow
```

## Build System Rules

CMake is the only supported build system for aria2-next. Do not restore Autotools files, introduce a second build system, or route maintained packaging through upstream aria2 build scripts.

Use target-oriented CMake and existing helper patterns from `CMakeLists.txt`. Keep generated values in `cmake/*.in` templates and generated build directories. Do not commit generated CMake, Ninja, CTest, CPack, binary, archive, or local build output.

Feature probes must match the code they guard. If a probe controls a compile-time branch, prefer `check_symbol_exists`, `check_cxx_symbol_exists`, or a small `check_*_source_compiles` snippet with the exact headers required by the source. Avoid naked `check_function_exists` when the code depends on headers, macros, platform-specific types, or cross-compilation behavior.

Windows and MinGW probes need extra care. Validate Win32 constants and APIs through compilable snippets with the right include order, such as `winsock2.h` before `windows.h`, and include dedicated headers such as `winioctl.h` when the source needs them. POSIX-only calls such as `fork`, `setsid`, and daemonization paths must not be enabled for Windows builds.

## Version Management

`CMakeLists.txt` is the single source of truth for the project version:

```cmake
project(
  aria2
  VERSION 2.0.0
  ...
)
```

`PROJECT_VERSION` feeds generated package metadata through CMake, including `PACKAGE_VERSION`, `VERSION`, and `cmake/libaria2.pc.cmake.in`. Helper scripts that need the project version must read it from `CMakeLists.txt`; they must not carry their own version constants.

Release artifact names derive from the GitHub Release tag passed to `.github/workflows/release.yml` as `RELEASE_TAG`. For a release tag `v2.0.0`, packaged artifacts must use `2.0.0` as the version component.

When bumping the project version, update the CMake project version first, then verify every generated or scripted consumer still derives from that source. Do not scatter manual version edits across scripts, Dockerfiles, workflow files, documentation, or package templates.

If a release version has already been tagged, treat that tag as immutable. Fixes after a failed release require a new commit and a deliberate tag recovery flow, not rebuilding different source under an unchanged published tag.

## Dependency Management

`packaging/dependencies.env` is the authoritative dependency version source for maintained release packaging. Update it before changing dependency versions in scripts, Dockerfiles, workflow files, platform notes, or release documentation.

Current maintained release dependencies include zlib, Expat, SQLite, c-ares, libssh2, OpenSSL, GMP, libgpg-error, libgcrypt, and the Android NDK. Keep the dependency table in `README.md`, packaging scripts, Docker build arguments, and workflow download/build steps consistent with `packaging/dependencies.env`.

Do not add automated dependency PR systems, scheduled dependency update workflows, or Dependabot configuration unless explicitly requested. Dependency updates should be intentional, reviewed, and verified through the affected release path.

## Release Process

The release workflow is `.github/workflows/release.yml`. It runs on `release: published` and on manual `workflow_dispatch` validation. Tag pushes must not trigger release builds directly.

Maintained release targets are:

| Platform | Job | Artifact |
| --- | --- | --- |
| Linux x86_64 | `build-linux-x64` | `aria2-<version>-linux-x86_64.tar.xz` |
| Linux ARM64 | `build-linux-arm64` | `aria2-<version>-linux-aarch64.tar.xz` |
| macOS ARM64 | `build-macos-arm64` | `aria2-<version>-macos-arm64.tar.bz2` |
| macOS x86_64 | `build-macos-intel` | `aria2-<version>-macos-x86_64.tar.bz2` |
| Windows x86_64 | `build-windows-x64` | `aria2-<version>-windows-x86_64.zip` |
| Windows ARM64 | `build-windows-arm64` | `aria2-<version>-windows-arm64.zip` |

The upload job downloads all platform artifacts, generates `aria2-<version>-checksums.sha256`, and uploads assets to the already-published GitHub Release with `gh release upload`. Manual workflow runs upload artifacts only to the Actions run for validation.

### Publishing a Release

Finish all code, packaging, documentation, and verification work before publishing the GitHub Release.

Use `v{PROJECT_VERSION}` as the release tag. The tag version and the `project(... VERSION ...)` value in `CMakeLists.txt` must match exactly after removing the leading `v`.

Before creating the GitHub Release, verify the local build:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2c --version
```

For release or packaging changes, also validate the touched shell scripts and workflow-adjacent files:

```bash
bash -n tools/build_test.sh
bash -n packaging/scripts/mingw-release
bash -n packaging/scripts/android-release
```

Push the verified release commit to `main` and wait for CI to pass. Then create and publish the GitHub Release. GitHub Release creation may create the tag; do not push the release tag separately as a release trigger.

```bash
git push origin main
gh release create v2.0.0 --title "v2.0.0 - CMake-Only Cross-Platform Release Foundation" --notes-file /path/to/release-notes.md
```

After the release workflow completes, inspect all six uploaded artifacts and the checksum file on the published GitHub Release.

### Failed Release Recovery

If the release workflow fails after the GitHub Release is published, fix the issue in a new commit. If the release has not been announced or consumed, delete the failed GitHub Release and tag, then recreate the same release from the fixed commit. If the release has already been consumed publicly, create a new patch release unless the maintainer explicitly chooses a tag replacement recovery.

```bash
gh release delete v2.0.0 --cleanup-tag --yes
git fetch --prune --tags
gh release create v2.0.0 --target main --title "v2.0.0 - CMake-Only Cross-Platform Release Foundation" --notes-file /path/to/release-notes.md
```

## CI and Verification

Use the smallest verification set that covers the changed surface, then expand when the change touches shared build logic, platform probes, or packaging.

For normal source or CMake changes:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

For broad CMake option changes, run the matrix helper:

```bash
tools/build_test.sh
```

For release workflow changes, validate the affected shell snippets, inspect the release matrix, and use `workflow_dispatch` with an explicit tag input when local verification cannot cover the platform path.

Do not ignore release test failures by adding `|| true` to new test commands. Existing tolerated checks should stay limited to diagnostics such as binary dependency inspection unless a maintainer explicitly accepts the risk.

## Source Conventions

Follow the existing C and C++ style. Keep C++ at the repository's C++11 baseline unless the project intentionally raises the baseline. Avoid unrelated refactors, broad formatting churn, and drive-by rewrites.

Prefer structured fixes over compatibility macros scattered through source files. Platform differences should usually be represented through accurate CMake detection and existing config-header patterns.

Use existing abstractions and naming. Add a helper only when it removes real duplication or keeps platform-specific behavior contained.

Changes under `third_party/` must preserve upstream layout and style. Limit third-party edits to build integration, security fixes, or compatibility fixes that cannot reasonably wait for upstream.

## Tests

Add focused tests for behavior changes when the existing test framework can express the case. CMake-only and packaging-only changes still need concrete build or workflow validation.

When fixing a platform build failure, record the failed symbol, header, macro, or branch condition in the reasoning for the change. The final fix should make the local configuration result match the code path that will compile on that platform.

## Documentation and Maintenance

Keep documentation synchronized with behavior. If a CMake option, dependency baseline, supported platform, artifact name, or release command changes, update the relevant README or packaging note in the same change.

`maintenance/` is for durable project records only. Do not commit temporary API payloads, scratch logs, generated reports, or local caches there.

Release history belongs in git tags and GitHub Releases. Do not add standalone changelog snapshots to packages or maintenance docs unless they are generated release artifacts.

## Git Safety

Work on the current branch unless the user explicitly asks for a new branch. Never revert user changes or unrelated work. If the tree is dirty, inspect the relevant files and make the smallest compatible change.

Do not use destructive git commands such as `git reset --hard` or `git checkout -- <path>` unless the user explicitly requests that exact operation.
