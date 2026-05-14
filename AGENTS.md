# AGENTS.md - aria2-next

This file defines repository rules for AI coding agents. Human contributors should start with [README.md](README.md), [packaging/README.md](packaging/README.md), [tools/README.md](tools/README.md), [third_party/README.md](third_party/README.md), and [docs/maintenance/README.md](docs/maintenance/README.md).

> [!IMPORTANT]
> All changes must meet industrial-grade quality. Find the root cause before changing code, keep behavior compatible unless the task explicitly changes it, avoid unrelated churn, and verify the exact build or release path affected by the change.

## Architecture

| Area | Ownership |
| --- | --- |
| Core | C99 and C++11 aria2 command-line client and core implementation |
| Build | CMake 3.25+ with Ninja as the default generator |
| Tests | CTest plus the CppUnit test suite |
| Packaging | Cross-platform release automation under `packaging/` and `.github/workflows/release.yml` |
| Third-party source | Vendored `third_party/wslay` with local ownership rules |

## Key Paths

| Path | Purpose |
| --- | --- |
| `CMakeLists.txt` | Project declaration and CMake module entry point |
| `CMakePresets.json` | Standard configure, build, and test presets |
| `cmake/modules/` | Build options, probes, dependencies, targets, tests, and summary output |
| `cmake/Sources.cmake` | Core source inventory |
| `cmake/TestSources.cmake` | Test source inventory |
| `src/` | aria2 core, CLI, protocol, disk, RPC, and platform code |
| `src/includes/aria2/` | Public libaria2 headers |
| `tests/` | CppUnit tests and fixtures |
| `docs/` | Manual sources, completion tooling, and maintenance records |
| `packaging/` | Release dependencies, Dockerfiles, cross-build scripts, and package assets |
| `packaging/dependencies.env` | Release dependency version source |
| `third_party/` | Bundled third-party source |
| `tools/` | Local developer helpers |

## Build Rules

CMake is the only supported build system for aria2-next. Do not restore Autotools files, add another maintained build system, or route release packaging through removed upstream build scripts.

Keep the top-level `CMakeLists.txt` small. New build logic belongs in the focused modules under `cmake/modules/`. Keep generated config headers, package metadata, CMake files, Ninja files, test output, binaries, and archives out of the source tree.

Feature probes must match the source they guard. Prefer `check_symbol_exists`, `check_cxx_symbol_exists`, or small `check_*_source_compiles` snippets with the exact headers needed by the compiled branch. Windows and MinGW probes must include the correct Win32 headers and include order.

## Version Management

`CMakeLists.txt` is the single source of truth for the project version:

```cmake
project(
  aria2
  VERSION 2.0.0
  ...
)
```

`PROJECT_VERSION` feeds generated package metadata and release artifact naming. Scripts that need the version must read it from `CMakeLists.txt`; they must not carry independent version constants.

Release tags use `v{PROJECT_VERSION}`. The tag version and CMake project version must match exactly after removing the leading `v`.

Treat published release tags as immutable. If a failed release has not been consumed, delete the failed GitHub Release and tag, fix the commit, then recreate the same release deliberately. If a release has been consumed publicly, publish a new patch release unless the maintainer explicitly chooses tag replacement.

## Dependency Management

`packaging/dependencies.env` is the authoritative dependency baseline for maintained release packaging. Update it before changing dependency versions in scripts, Dockerfiles, workflow files, package notes, or README tables.

Do not add automated dependency PR systems, scheduled dependency update workflows, or Dependabot configuration unless explicitly requested. Dependency updates should be intentional and verified through the affected release path.

## Release Process

The release workflow is `.github/workflows/release.yml`. It runs on `release: published` and on manual `workflow_dispatch` validation. Tag pushes do not publish release builds directly.

Maintained release artifacts are Linux x86_64, Linux ARM64, macOS ARM64, macOS x86_64, Windows x86_64, Windows ARM64, and `aria2-<version>-checksums.sha256`.

Manual workflow runs are for release-path validation. Official release assets are uploaded only when a GitHub Release is published.

Before publishing a GitHub Release, verify locally:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2c --version
```

For packaging changes, also run:

```bash
bash -n tools/build_test.sh
bash -n packaging/scripts/common.sh
bash -n packaging/scripts/mingw-release
bash -n packaging/scripts/android-release
```

## CI and Verification

Use the smallest verification set that covers the changed surface, then expand when changes touch shared build logic, platform probes, or packaging.

Normal source or CMake changes require:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
```

Broad CMake option changes require the local matrix helper:

```bash
tools/build_test.sh
```

Do not hide new test failures with `|| true`. Existing tolerated diagnostics must stay limited to non-gating inspection commands unless the maintainer explicitly accepts the risk.

## Source Conventions

Follow the existing C and C++ style. Keep C++ at the repository's C++11 baseline unless the project intentionally raises the baseline. Avoid drive-by rewrites and broad formatting churn.

Prefer accurate CMake detection and existing config-header patterns over scattered compatibility macros. Add helpers only when they remove real duplication or contain platform-specific behavior cleanly.

Changes under `third_party/` must preserve third-party ownership. Limit edits there to build integration, security fixes, or compatibility fixes that cannot reasonably wait for upstream.

## Documentation

Keep documentation synchronized with behavior. If a CMake option, dependency baseline, supported platform, artifact name, release command, or directory path changes, update the relevant documentation in the same change.

`docs/maintenance/` is for durable audit records only. Do not commit temporary API payloads, scratch logs, generated reports, or local caches there.

## Git Safety

Work on the current branch unless the user explicitly asks for a new branch. Never revert user changes or unrelated work. Do not use destructive git commands unless the user explicitly requests that exact operation.
