# Contributing to Aria2 Next

Aria2 Next is an aria2-compatible download engine. Contributions must preserve the command-line, configuration, session, and JSON-RPC surfaces unless a change deliberately adds an aria2-next extension.

## Development Setup

Install CMake 3.25 or newer, Ninja, pkg-config, a C99/C++17 compiler, CppUnit, libcurl, OpenSSL, Boost, libtorrent-rasterbar, and zlib.

```bash
git clone https://github.com/AnInsomniacy/aria2-next.git
cd aria2-next
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2-next --version
```

The default preset writes to `build/default/`. Do not commit build directories, generated CMake files, binaries, archives, logs, dumps, or release artifacts.

## Project Boundaries

The maintained public surface is the `aria2-next` executable, aria2-style configuration files, session files, input files, and JSON-RPC. There is no supported public C++ embedding API.

Motrix Next consumes aria2-next as a sidecar through CLI options and JSON-RPC. UI behavior, app preferences, installers, tray behavior, history, notifications, and auto-update UI belong in Motrix Next. Engine crashes, transfer failures, wrong RPC fields, protocol errors, release binary problems, and build failures belong here.

The maintained protocol surface includes HTTP, HTTPS, FTP, SFTP, BitTorrent, magnet, ED2K, checksums, session files, input files, and JSON-RPC. BitTorrent is owned by libtorrent-rasterbar. ED2K is native aria2-next code.

## Code Rules

Keep changes narrow and evidence-based. Find the root cause before editing. Avoid unrelated formatting, broad rewrites, and opportunistic dependency updates.

Use the existing C and C++ style. The baseline is C99 and C++17. Prefer local helpers with clear ownership over shared abstractions that hide protocol behavior.

CMake is the only supported build system. New build logic belongs under `cmake/modules/`; keep the top-level `CMakeLists.txt` small.

## Testing

Use the smallest test that proves the changed behavior. Expand only when the change touches shared transfer, storage, RPC, build, or release behavior.

Normal source changes require:

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2-next --version
```

Packaging or release changes also require:

```bash
bash -n tools/build_test.sh
bash -n scripts/bump-version.sh
bash -n scripts/release.sh
bash -n packaging/scripts/common.sh
bash -n packaging/scripts/mingw-release
bash -n packaging/scripts/android-release
```

Broad CMake option changes should run:

```bash
tools/build_test.sh
```

Do not hide failures with `|| true`. If a check cannot be run locally, explain the reason in the PR.

## Bug Fixes

Add a regression test when the behavior can be covered locally. Network-specific bugs should usually become parser tests, local-server tests, command-level tests, or protocol fixtures.

When a remote service triggers the issue, document the observable behavior that matters, such as ignored HTTP Range requests, invalid `Content-Range`, expired signed URLs, certificate chain failures, proxy environment leakage, torrent metadata timing, or ED2K source availability.

## Features and Extensions

Open an issue before implementing new CLI options, JSON-RPC fields, protocol behavior, or release artifact changes.

JSON-RPC extensions must be explicit and stable. Do not put placeholders into existing aria2 fields. Prefer a small new field with precise semantics.

## Dependencies

`packaging/dependencies.env` owns maintained release dependency versions, source archive URLs, archive names, and SHA-256 hashes. Update it before changing dependency versions in scripts, Dockerfiles, workflows, package notes, or README tables.

Dependency changes must name the upstream version, source URL, SHA-256 hash, and affected release path.

## Pull Requests

Each PR should address one concern. Keep behavior changes, refactors, dependency updates, and documentation updates separate unless they are all required for one fix.

PRs must explain the affected surface, compatibility impact, verification commands, and release-note impact. The pull request template contains the required checklist.

Use Conventional Commits:

```text
fix(http): retry transient segmented transfer failures
feat(rpc): expose ED2K visible progress
docs: tighten troubleshooting guidance
ci(release): add Windows debug artifacts
```

AI-assisted development is allowed when the author reviews, understands, tests, and can explain the submitted changes. Fill out the PR disclosure honestly.

## Security

Do not publish secrets, private cookies, proxy credentials, signed URL tokens, private tracker data, exploit details, or unredacted crash dumps in public issues. Follow [`SECURITY.md`](SECURITY.md).
