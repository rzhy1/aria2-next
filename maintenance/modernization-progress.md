# Modernization Progress

## 2026-05-14T00:48:47Z - Checkpoint 1

Changed:

- Moved platform Dockerfiles into `packaging/docker/`.
- Moved the unsupported Raspberry Pi Trusty Dockerfile into `packaging/legacy/`.
- Moved the retired Travis CI configuration into `packaging/legacy/`.
- Moved Android dependency helper configuration scripts into `packaging/android/`.
- Moved release and cross-build helper scripts into `packaging/scripts/`.
- Moved the macOS package makefile and package resources into `packaging/macos/`.
- Moved general helper scripts into `tools/`.
- Moved bundled `wslay` source into `third_party/wslay`.
- Updated autotools paths for `third_party/wslay` and the new `third_party/Makefile.am` entry point.
- Updated release helper paths so maintained Docker builds use the current repository instead of cloning upstream aria2.
- Updated the main README files to describe the aria2-next layout and moved helper script locations.

Verified:

- Inspected the updated root directory layout.
- Checked for stale `deps/wslay`, `deps/Makefile`, `osx-package`, and root Dockerfile references.

Remaining:

- Clean ignored generated files and update `.gitignore` for the new layout.
- Centralize dependency versions and refresh maintained dependency baselines.
- Finish legacy packaging quarantine documentation.
- Move documentation theme handling to pinned Python documentation dependencies.
- Run full autotools regeneration, build, and test verification after all checkpoints.

## 2026-05-14T00:50:05Z - Checkpoint 2

Changed:

- Removed local autotools, libtool, compiler, documentation, and test runtime outputs from the working tree.
- Removed generated `wslay` build outputs left behind by the old `deps/wslay` location.
- Updated `.gitignore` so generated `wslay` files are ignored under `third_party/wslay`.
- Added packaging output ignore patterns for maintained release helper output directories.

Verified:

- Re-ran `git status --short --ignored` to check that remaining generated files are either removed or ignored.

Remaining:

- Centralize dependency versions and refresh maintained dependency baselines.
- Finish legacy packaging quarantine documentation.
- Move documentation theme handling to pinned Python documentation dependencies.
- Run full autotools regeneration, build, and test verification after all checkpoints.

## 2026-05-14T00:55:27Z - Checkpoint 3

Changed:

- Added `packaging/dependencies.env` as the authoritative release dependency metadata file.
- Updated maintained dependency baselines to zlib 1.3.2, Expat 2.8.1, SQLite 3.53.1, c-ares 1.34.6, libssh2 1.11.1, OpenSSL 3.5.6 LTS, libgpg-error 1.61, libgcrypt 1.12.2, and Android NDK r29.
- Kept GMP at 6.3.0 because it remains the current stable release.
- Updated GitHub Actions download and build steps to source `packaging/dependencies.env`.
- Updated maintained Dockerfiles to accept dependency versions as build arguments.
- Updated README dependency tables and Android/Windows dependency notes.

Verified:

- Checked official release metadata for OpenSSL, zlib, Expat, SQLite, c-ares, libssh2, libgcrypt, libgpg-error, and Android NDK before choosing versions.
- Searched maintained documentation, packaging scripts, Dockerfiles, and release workflow for stale dependency version strings.

Remaining:

- Finish legacy packaging quarantine documentation.
- Move documentation theme handling to pinned Python documentation dependencies.
- Run full autotools regeneration, build, and test verification after all checkpoints.

## 2026-05-14T00:56:08Z - Checkpoint 4

Changed:

- Added `packaging/legacy/README.md` to mark unsupported historical packaging explicitly.
- Kept the Raspberry Pi Trusty Dockerfile only as a legacy reference under `packaging/legacy/`.
- Updated the maintained MinGW Dockerfile default host to x86_64 while preserving 32-bit override support.
- Verified maintained Dockerfiles build the current repository context instead of cloning upstream aria2.
- Documented the legacy packaging location in `README.md`.

Verified:

- Searched maintained packaging and release files for remaining upstream clone paths and root Dockerfile references.
- Confirmed stale Raspberry Pi assumptions remain only inside `packaging/legacy/`.

Remaining:

- Move documentation theme handling to pinned Python documentation dependencies.
- Run full autotools regeneration, build, and test verification after all checkpoints.

## 2026-05-14T00:56:40Z - Checkpoint 5

Changed:

- Added `third_party/README.md` to document bundled third-party ownership.
- Documented that bundled `wslay` remains vendored because 1.1.1 is still the current upstream release.
- Documented `--with-system-wslay` as future work rather than part of this cleanup.
- Linked the third-party policy from `README.md`.

Verified:

- Searched autotools files and ignore rules for stale `deps/wslay` references.
- Confirmed `configure.ac` now points WebSocket subconfiguration, include paths, and libtool paths at `third_party/wslay`.

Remaining:

- Move documentation theme handling to pinned Python documentation dependencies.
- Run full autotools regeneration, build, and test verification after all checkpoints.

## 2026-05-14T01:04:03Z - Checkpoint 6

Changed:

- Added `doc/requirements.txt` as the pinned Python documentation dependency entry point.
- Removed local `html_theme_path` overrides from all Sphinx configuration templates.
- Updated documentation build instructions to install `doc/requirements.txt` before building manuals.
- Added `doc/requirements.txt` to the distribution file list.
- Removed the vendored Sphinx Read the Docs theme copy from `doc/sphinx_themes`.

Verified:

- Inspected all language Sphinx configuration templates for local theme path overrides.
- Checked documentation makefiles to confirm they continue to call `sphinx-build` directly.

Remaining:

- Run full autotools regeneration, build, and test verification.

## 2026-05-14T01:09:46Z - Checkpoint 7

Changed:

- Fixed SQLite cookie parsing so read-only WAL snapshots retry through the immutable SQLite URI path when the initial read-only handle opens but cannot read the database.
- Updated Android Docker release arguments so dependency tag and year metadata come from `packaging/dependencies.env`.
- Corrected the Android Dockerfile `RANLIB` environment variable.
- Updated translation helper usage text after moving `tools/import-po`.

Verified:

- Ran `autoreconf -i`; it completed with only the existing obsolete `AC_PROG_GCC_TRADITIONAL` warning.
- Ran `./configure`; it completed and configured `third_party/wslay`.
- Ran `make -j$(sysctl -n hw.ncpu || nproc || echo 2)`; it completed and built `src/aria2c`.
- Ran `./aria2c` from `test/`; it passed 1009 CppUnit checks.
- Ran `make check -j$(sysctl -n hw.ncpu || nproc || echo 2)`; it passed with `# TOTAL: 1`, `# PASS: 1`, `# FAIL: 0`, `# ERROR: 0`.
- Ran `bash -n` over maintained shell scripts in `packaging/scripts/` and `tools/`.
- Parsed `.github/workflows/release.yml` with PyYAML and confirmed all release jobs are present.
- Searched for stale root packaging paths, stale `deps/wslay` references, vendored Sphinx theme references, and the old Android `RANDLIB` typo.

Remaining:

- None for this modernization goal.
