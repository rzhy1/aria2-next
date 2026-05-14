# aria2-next

aria2-next is a maintained fork of [aria2](https://github.com/aria2/aria2).

It exists to provide a reliable, modern, reproducible aria2 engine for
[Motrix Next](https://github.com/AnInsomniacy/motrix-next) and for other users
who need current cross-platform aria2 builds. The command-line program remains
`aria2c`, and the project preserves aria2-compatible CLI options, configuration
files, session files, JSON-RPC behavior, package metadata, and libaria2 surfaces
unless a change is explicitly documented.

This is not a new downloader with a different protocol model. It is an aria2
fork with active maintenance, CMake-only builds, updated dependency baselines,
reviewed bug fixes, and automated release packaging for the platforms Motrix
Next ships on.

## Why This Fork Exists

Upstream aria2 is stable and widely used, but modern application distribution
needs more than the historical release surface. Motrix Next embeds aria2 as a
Tauri sidecar and needs signed, reproducible, current binaries for macOS,
Windows, and Linux on both x64 and ARM64.

aria2-next focuses on four maintenance goals:

- Preserve aria2 behavior and compatibility for existing users.
- Fix reviewed upstream issues with targeted tests where the source defect is
  actionable.
- Maintain a single CMake release path instead of the former Autotools build.
- Produce repeatable multi-platform release artifacts from GitHub Actions.

## Compatibility Contract

The default executable is still `aria2c`.

Existing aria2 integrations should continue to use the same command-line flags,
configuration files, input files, RPC methods, session files, and output
conventions. Motrix Next consumes this project as a sidecar engine, but the
release binaries are ordinary aria2-compatible binaries and can be used outside
Motrix Next.

Compatibility-sensitive changes must be intentional, tested, and documented.
Packaging names may use the aria2-next repository identity, but runtime behavior
should stay compatible with aria2 unless there is a clear maintenance reason to
change it.

## Maintenance Audit

The `maintenance/` directory records the durable review work behind this fork.
The authoritative artifact is
[`maintenance/issue-review-matrix.csv`](maintenance/issue-review-matrix.csv).

The preserved matrix contains 137 upstream open bug issues that survived the
bug-focused cleanup pass. Each retained issue was reviewed with a final state,
root-cause group, required action, and evidence. Current preserved results
include 44 issues in a `fixed-verified` state and 37 issues whose required
action was `fixed-verified`.

The matrix is intentionally conservative. Some upstream reports were fixed in
code, some were already fixed, some matched documented behavior, and some were
classified as environment, site, platform, or architecture limitations. Reports
without enough current evidence were retained as review records rather than
being converted into speculative patches.

## CMake-Only Build System

CMake is the only supported build system in this repository. Ninja is the
default generator used by local development and release automation.

Autotools files and legacy packaging paths were removed from the maintained
tree. Third-party dependencies may still use their own upstream build systems
when they are built as release dependencies, but aria2-next itself must be
configured and built through CMake.

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2c --version
```

A plain Ninja build without presets is also supported:

```bash
cmake -S . -B build/default -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build/default
ctest --test-dir build/default --output-on-failure
```

Useful CMake options include `ARIA2_ENABLE_BITTORRENT`,
`ARIA2_ENABLE_METALINK`, `ARIA2_ENABLE_WEBSOCKET`, `ARIA2_ENABLE_LIBARIA2`,
`ARIA2_ENABLE_STATIC`, `ARIA2_WITH_APPLETLS`, `ARIA2_WITH_WINTLS`,
`ARIA2_WITH_OPENSSL`, `ARIA2_WITH_GNUTLS`, `ARIA2_WITH_LIBXML2`,
`ARIA2_WITH_EXPAT`, `ARIA2_WITH_CARES`, `ARIA2_WITH_ZLIB`,
`ARIA2_WITH_SQLITE3`, and `ARIA2_WITH_LIBSSH2`.

## Supported Release Targets

| Platform | Architecture | TLS backend | Release artifact |
| --- | --- | --- | --- |
| Linux | x86_64 | OpenSSL | `aria2-<version>-linux-x86_64.tar.xz` |
| Linux | ARM64 | OpenSSL | `aria2-<version>-linux-aarch64.tar.xz` |
| macOS | Apple Silicon | AppleTLS or OpenSSL | `aria2-<version>-macos-arm64.tar.bz2` |
| macOS | Intel | AppleTLS or OpenSSL | `aria2-<version>-macos-x86_64.tar.bz2` |
| Windows | x86_64 | Schannel or OpenSSL | `aria2-<version>-windows-x86_64.zip` |
| Windows | ARM64 | Schannel or OpenSSL | `aria2-<version>-windows-arm64.zip` |
| Android | ARM64 | OpenSSL | Maintained packaging path |

Release packaging and cross-compilation assets live under `packaging/`.
The authoritative release dependency versions are stored in
[`packaging/dependencies.env`](packaging/dependencies.env).

## Release Model

`CMakeLists.txt` is the project version source of truth:

```cmake
project(
  aria2
  VERSION 2.0.0
  ...
)
```

Release tags must use `v{PROJECT_VERSION}`. For example, CMake version `2.0.0`
maps to tag `v2.0.0`.

CI and release automation have separate responsibilities:

- `.github/workflows/ci.yml` validates push and pull request changes with a
  CMake configure, build, test, binary version check, and maintained shell script
  syntax checks.
- `.github/workflows/release.yml` runs when a GitHub Release is published. It
  validates that the release tag matches the CMake project version, builds all
  maintained platform artifacts, generates SHA-256 checksums, and uploads the
  assets to the published GitHub Release.
- `workflow_dispatch` remains available for manual release-path validation. It
  uploads artifacts to the workflow run only. It is not the normal publishing
  path.

A tag push alone is not the release trigger. The formal release trigger is the
published GitHub Release.

## Dependency Baseline

| Dependency | Version | Release usage |
| --- | --- | --- |
| zlib | 1.3.2 | All release targets |
| Expat | 2.8.1 | Release targets using Expat |
| c-ares | 1.34.6 | Async DNS release targets |
| SQLite | 3.53.1 | Cookie storage release targets |
| libssh2 | 1.11.1 | SFTP release targets |
| OpenSSL | 3.5.6 LTS | Linux, Windows, Android, optional macOS |
| GMP | 6.3.0 | macOS and Windows dependency builds |
| libgpg-error | 1.61 | macOS dependency builds |
| libgcrypt | 1.12.2 | macOS dependency builds |
| Android NDK | r29 | Android release build |

Update `packaging/dependencies.env` before changing dependency versions in
scripts, Dockerfiles, workflow files, platform notes, or release documentation.

## Repository Layout

| Path | Purpose |
| --- | --- |
| `CMakeLists.txt` | Root CMake build definition and project version source |
| `CMakePresets.json` | Standard configure, build, and test presets |
| `cmake/` | CMake templates, source inventories, and generated config inputs |
| `src/` | aria2 command-line client and core implementation |
| `src/includes/aria2/` | public libaria2 headers |
| `test/` | CppUnit test suite registered through CTest |
| `doc/` | manual, manpage, bash completion, and documentation tooling |
| `packaging/` | release dependencies, Dockerfiles, cross-build scripts, package assets |
| `third_party/` | vendored source with explicit ownership rules |
| `tools/` | repository helper scripts outside platform packaging |
| `maintenance/` | issue review records and CMake migration notes |

See [`packaging/README.md`](packaging/README.md),
[`tools/README.md`](tools/README.md), [`third_party/README.md`](third_party/README.md),
and [`maintenance/README.md`](maintenance/README.md) for directory-specific
rules.

## Maintenance Rules

Keep source behavior, CMake options, packaging, dependency metadata, and
documentation synchronized. If a build option, supported platform, dependency
version, artifact name, or release command changes, update the corresponding
documentation in the same change.

Release packaging must build this repository checkout through CMake. It must not
clone upstream aria2 during a release build.

Unsupported packaging experiments should be removed unless they have a current
maintainer and a verified CMake release path.

Changes under `third_party/` should preserve upstream layout and style. Limit
local edits there to build integration, security fixes, or compatibility fixes
that cannot reasonably wait for upstream.

## License

Same as [aria2](https://github.com/aria2/aria2): [GPLv2](COPYING).
