# Windows Package Note

This package contains the aria2-next `aria2c.exe` binary for Windows. It is statically linked for portable use and keeps the aria2 command-line and RPC interfaces intact.

Maintained dependency versions are defined in `packaging/dependencies.env` in the source tree.

Statically linked release dependencies:

| Dependency | Version |
| --- | --- |
| GMP | 6.3.0 |
| Expat | 2.8.1 |
| SQLite | 3.53.1 |
| zlib | 1.3.2 |
| c-ares | 1.34.6 |
| libssh2 | 1.11.1 |
| OpenSSL | 3.5.6 LTS |

Example use from PowerShell:

```powershell
.\aria2c.exe --version
.\aria2c.exe https://example.com/file.iso
```

`--daemon` is not supported on Windows. Use a service manager, scheduler, or parent application when background process management is required.
