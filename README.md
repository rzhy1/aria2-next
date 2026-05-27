<div align="center">
  <img src="docs/media/aria2-next-icon.png" alt="Aria2 Next icon" width="144" height="144" />
  <h1>Aria2 Next</h1>
  <p>Maintained aria2-compatible download engine with modern transfer backends and cross-platform release builds.</p>

[![CI](https://github.com/AnInsomniacy/aria2-next/actions/workflows/ci.yml/badge.svg)](https://github.com/AnInsomniacy/aria2-next/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/AnInsomniacy/aria2-next.svg)](https://github.com/AnInsomniacy/aria2-next/releases)
[![License: GPLv2](https://img.shields.io/badge/license-GPLv2-blue.svg)](COPYING)

  <p>
    <img src="https://img.shields.io/badge/platform-macOS%20%7C%20Windows%20%7C%20Linux%20%7C%20Android-blue.svg" alt="Platform: macOS, Windows, Linux, Android" />
  </p>
</div>

## Overview

Aria2 Next is a maintained fork of [aria2](https://github.com/aria2/aria2). It preserves the familiar executable, command-line options, configuration files, session files, input files, and JSON-RPC surface while modernizing the implementation and release pipeline.

The main executable is `aria2-next`. It can be used directly from scripts and terminals, and it is also the download engine bundled by [Motrix Next](https://github.com/AnInsomniacy/motrix-next).

Current maintenance focuses on reliable HTTP/HTTPS transfers through libcurl, BitTorrent and magnet handling through libtorrent-rasterbar, native ED2K support, stable JSON-RPC behavior, reproducible release artifacts, and current dependency baselines.

## Supported Surface

| Area | Status |
| --- | --- |
| Executable | `aria2-next` |
| CLI and config | aria2-compatible option names and file formats |
| Sessions and input files | aria2-compatible session and input-file conventions |
| JSON-RPC | aria2-compatible methods with explicit aria2-next extension fields where needed |
| Protocols | HTTP, HTTPS, FTP, SFTP, BitTorrent, magnet, ED2K |
| Primary app integration | Motrix Next sidecar engine |
| Public C++ API | Not maintained |

## Quick Start

Download a file:

```bash
aria2-next https://example.com/file.iso
```

Download an ED2K file link:

```bash
aria2-next '<ed2k-file-link>'
```

Run the JSON-RPC server:

```bash
aria2-next --enable-rpc=true --rpc-listen-all=false --rpc-listen-port=6800
```

Inspect enabled features:

```bash
aria2-next --version
aria2-next --help=#all
```

## Downloads

Prebuilt binaries are published on [GitHub Releases](https://github.com/AnInsomniacy/aria2-next/releases).

| Platform | Architecture | Asset |
| --- | --- | --- |
| Linux | x86_64 | `aria2-next-<version>-linux-x86_64` |
| Linux | ARM64 | `aria2-next-<version>-linux-aarch64` |
| macOS | Apple Silicon | `aria2-next-<version>-macos-arm64` |
| macOS | Intel | `aria2-next-<version>-macos-x86_64` |
| Windows | x86_64 | `aria2-next-<version>-windows-x86_64.exe` |
| Windows | ARM64 | `aria2-next-<version>-windows-arm64.exe` |
| Checksums | all assets | `aria2-next-<version>-checksums.sha256` |

Linux and macOS binaries are executable files. If the executable bit is missing:

```bash
chmod +x ./aria2-next-<version>-<platform>
```

Release binaries verify HTTPS certificates by default. Windows uses the native Windows certificate store through libcurl. macOS uses Apple SecTrust. Linux uses libcurl/OpenSSL CA auto-discovery with OpenSSL fallback paths. Android shell environments may need `--ca-certificate`.

Checksum verification, signing status, debug artifacts, and release rules are documented in [`docs/RELEASE.md`](docs/RELEASE.md).

## Build

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2-next --version
```

The default preset uses CMake and Ninja. Common build options include `ARIA2_ENABLE_BITTORRENT`, `ARIA2_ENABLE_WEBSOCKET`, `ARIA2_ENABLE_STATIC`, `ARIA2_RELEASE_SIZE_OPTIMIZED`, `ARIA2_RELEASE_LTO`, and `ARIA2_WITH_ZLIB`.

Release dependency versions, source URLs, archive names, and SHA-256 hashes are maintained in [`packaging/dependencies.env`](packaging/dependencies.env).

## Documentation

| Topic | Document |
| --- | --- |
| Contributor setup and PR rules | [`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md) |
| Troubleshooting and issue boundaries | [`docs/TROUBLESHOOTING.md`](docs/TROUBLESHOOTING.md) |
| App and JSON-RPC integration | [`docs/INTEGRATION.md`](docs/INTEGRATION.md) |
| Release assets and maintainer flow | [`docs/RELEASE.md`](docs/RELEASE.md) |
| Security reporting | [`docs/SECURITY.md`](docs/SECURITY.md) |
| Privacy and network behavior | [`docs/PRIVACY.md`](docs/PRIVACY.md) |
| Full documentation index | [`docs/README.md`](docs/README.md) |

Use GitHub issue forms for reproducible bugs, crashes, build and packaging problems, feature proposals, and focused usage questions.

## Maintenance Records

Durable audit records live under [`docs/maintenance/`](docs/maintenance/). They preserve the modernization history, upstream issue review, dependency decisions, ED2K work, and BitTorrent migration records. They are historical evidence, not the first place to learn normal usage.

## Repository Layout

| Path | Purpose |
| --- | --- |
| `CMakeLists.txt` | Project declaration and version source |
| `CMakePresets.json` | Standard configure, build, and test presets |
| `cmake/` | CMake modules, source inventories, and generated config templates |
| `src/` | aria2 command-line client and core implementation |
| `tests/` | CppUnit test suite registered through CTest |
| `docs/` | Documentation, manual sources, completion tooling, and maintenance records |
| `.github/` | Issue forms, pull request template, CI, and release workflows |
| `packaging/` | Release dependencies, cross-build scripts, Dockerfiles, and package assets |
| `third_party/` | Vendored source with explicit ownership rules |
| `tools/` | Local developer helpers |

## License

Same as [aria2](https://github.com/aria2/aria2): [GPLv2](COPYING). The OpenSSL linking exception text is preserved in [`docs/licenses/OPENSSL.md`](docs/licenses/OPENSSL.md).
