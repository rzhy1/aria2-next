# Android ARM64 Package Note

This package contains the aria2-next `aria2c` binary for Android ARM64. It is a native command-line executable, not an Android application package.

The binary is built with Android NDK r29. Maintained dependency versions are defined in `packaging/dependencies.env` in the source tree.

Statically linked release dependencies:

| Dependency | Version |
| --- | --- |
| OpenSSL | 3.5.6 LTS |
| Expat | 2.8.1 |
| zlib | 1.3.2 |
| c-ares | 1.34.6 |
| libssh2 | 1.11.1 |

Example use from an Android shell environment:

```sh
chmod 755 ./aria2c
./aria2c --version
./aria2c https://example.com/file.iso
```

Android certificate and DNS paths differ across shells and devices. Pass `--ca-certificate`, `--async-dns`, or `--async-dns-server` explicitly when your runtime environment requires them.
