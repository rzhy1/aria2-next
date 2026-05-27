# Troubleshooting

This guide covers common Aria2 Next engine failures, the boundary with Motrix Next, and the evidence needed for useful reports.

## Report Boundary

Use this repository for engine issues: process crashes, transfer failures, wrong JSON-RPC fields, CLI behavior regressions, protocol errors, certificate failures, proxy handling bugs, checksum failures, session/input-file regressions, release binary problems, and build failures.

Use the Motrix Next repository for UI behavior, app preferences, installers, auto-update UI, tray behavior, history database behavior, notifications, browser-extension flows, and desktop integration.

Motrix Next starts Aria2 Next as a sidecar and talks to it through JSON-RPC. If the same failure reproduces with the standalone engine, report it here. If the engine returns correct data but the app renders it incorrectly, report it in Motrix Next.

## First Checks

Start with the binary version and feature list:

```bash
aria2-next --version
aria2-next --help=#all
```

Run one failing task with debug logging:

```bash
aria2-next --no-conf --log-level=debug --console-log-level=debug --log=aria2-next.log <URI>
```

Redact RPC secrets, cookies, authorization headers, proxy credentials, private tracker URLs, signed URL tokens, and private local paths before sharing logs.

## HTTPS Certificates

Official Windows builds use the Windows certificate store through libcurl native CA support. Official macOS builds use Apple SecTrust. Official Linux builds use libcurl/OpenSSL CA auto-discovery and OpenSSL fallback paths. Android shell environments may need an explicit CA path.

For custom CA bundles:

```bash
aria2-next --ca-certificate=/path/to/ca-bundle.pem https://example.com/file
```

Certificate reports should include the exact OpenSSL verify result from the log. Common external causes include corporate TLS interception, missing system roots, proxy certificates, and expired server chains.

## Proxy Behavior

`--proxy-mode=auto` is the CLI default. It allows configured proxy options and environment proxy variables.

`--proxy-mode=direct` disables proxy use for HTTP, HTTPS, and FTP transfers.

`--proxy-mode=manual` uses only explicitly configured Aria2 Next proxy options such as `--all-proxy`, `--http-proxy`, and `--https-proxy`.

Applications with an explicit no-proxy setting should pass `--proxy-mode=direct`.

## HTTP Range and Large Downloads

Segmented HTTP downloads require valid byte-range responses. A valid ranged response uses `206 Partial Content`, a matching `Content-Range`, and identity encoding.

Some mirrors ignore Range and return `200 OK` with the full file. Aria2 Next detects this and can restart the task as a single full-body transfer instead of writing full-body data into a segment.

Large-download reports should include log lines containing `Range`, `Content-Range`, `HTTP range request`, `Connection timed out`, or `CURLE_WRITE_ERROR`.

## Checksums and Input Files

Input files can include per-task checksums:

```text
https://example.com/file.iso
  out=file.iso
  checksum=sha-1=0123456789abcdef0123456789abcdef01234567
```

Aria2 Next verifies whole-file checksums after transfer completion. If validation is skipped or `.aria2` files remain unexpectedly, include the input-file entry, command line, final console output, debug log, and resulting file/control-file state.

## Remote Timestamps

`-R` and `--remote-time=true` apply the server `Last-Modified` timestamp after a successful download.

If the timestamp is not applied, confirm the server returns `Last-Modified` and include the response headers or debug log.

## BitTorrent and Magnet

BitTorrent and magnet tasks use libtorrent-rasterbar. During magnet metadata download, JSON-RPC status exposes metadata state under `bittorrent.metadata`. Real torrent metadata appears under `bittorrent.info` only after metadata is available.

Magnet reports should include the magnet link or a redacted equivalent, `--pause-metadata`, the relevant `tellStatus` output, stderr, and whether the engine process stays alive.

## ED2K

ED2K downloads need reachable servers or inline sources. Low source availability can look like a stalled download even when the engine is running correctly.

ED2K reports should include the ED2K link, configured `--ed2k-server`, `--ed2k-server-list`, `--ed2k-node-list`, TCP/UDP listen ports, and debug log lines mentioning ED2K source discovery or peer transfer.

## Crashes

Windows crash reports should include the exception code, faulting module, fault offset, WER entry, startup command, trigger URI or RPC call, and whether the binary is an official release or debug workflow artifact.

Linux and macOS crash reports should include the signal, stderr, core dump notes when available, startup command, trigger URI or RPC call, and the final debug log lines.

Do not upload dumps containing secrets, cookies, private URLs, or credentials without redaction.
