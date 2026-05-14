# aria2-next

aria2-next is a maintained [aria2](https://github.com/aria2/aria2) fork focused on reliability fixes, current dependency baselines, and reproducible cross-platform builds.

## Supported Platforms

| Platform | Arch | SSL/TLS | Linking |
|----------|------|---------|:-------:|
| Linux | x86_64 | OpenSSL | ✅ Fully static |
| Linux | ARM64 | OpenSSL | ✅ Fully static |
| macOS | ARM64 | AppleTLS | ✅ System frameworks only |
| macOS | x86_64 | AppleTLS | ✅ System frameworks only |
| Windows | x86_64 | Schannel | ✅ System DLLs only |
| Windows | ARM64 | Schannel | ✅ System DLLs only |

> **Note:** Official aria2 releases only ship Windows (x86/x64) and Android (ARM64) binaries.

## Features

All builds include the full feature set:

- Async DNS, BitTorrent, Metalink, XML-RPC
- HTTPS, SFTP (via libssh2)
- GZip, Message Digest, Firefox3 Cookie

## Usage

Download the latest binary from [Releases](../../releases), extract, and run:

```bash
chmod +x aria2c   # Linux/macOS only
./aria2c --version
```

## Build Details

| Dependency | Version | Platforms |
|------------|---------|-----------|
| zlib | 1.3.2 | All |
| expat | 2.8.1 | All |
| c-ares | 1.34.6 | All |
| SQLite | 3.53.1 | All |
| libssh2 | 1.11.1 | All |
| OpenSSL | 3.5.6 LTS | Linux |
| GMP | 6.3.0 | macOS, Windows |
| libgpg-error | 1.61 | macOS |
| libgcrypt | 1.12.2 | macOS |

macOS binaries target `MACOSX_DEPLOYMENT_TARGET=11.0` (Big Sur+) for maximum compatibility.

## Origin

Built as part of the [Motrix Next](https://github.com/AnInsomniacy/motrix-next) project. Binaries are universal and can be used by any project or standalone.

## Repository Layout

The project keeps core source code in `src/`, tests in `test/`, documentation sources in `doc/`, bundled third-party source in `third_party/`, packaging and release assets in `packaging/`, maintenance records in `maintenance/`, helper scripts in `tools/`, build macros in `m4/`, translations in `po/`, and library support code in `lib/`.

Historical packaging files that are not part of the maintained release pipeline live under `packaging/legacy/`.
Bundled third-party code is documented in `third_party/README.md`.

## License

Same as [aria2](https://github.com/aria2/aria2) — [GPLv2](COPYING).
