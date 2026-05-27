# Integration Guide

This guide describes the supported integration surface for applications, scripts, and JSON-RPC clients that launch or control Aria2 Next.

## Stable Surfaces

The maintained public surfaces are the `aria2-next` executable, aria2-compatible command-line options, configuration files, session files, input files, and JSON-RPC.

There is no maintained public C++ embedding API. Parent applications should launch the executable and control it through CLI options and JSON-RPC.

## Compatibility Model

Aria2 Next preserves aria2-compatible behavior where practical. Extensions are added only when compatibility fields cannot safely represent modern engine state.

Clients should treat Aria2 Next extension fields as optional and ignore unknown fields. Clients should not infer state from placeholder names, empty objects, or file paths when explicit fields are available.

## Proxy Contract

`--proxy-mode=auto` allows configured proxy options and environment proxy variables. This is the command-line default.

`--proxy-mode=direct` disables proxy use for HTTP, HTTPS, and FTP transfers. Applications should pass it when the user selects no proxy.

`--proxy-mode=manual` ignores environment proxy variables and uses only explicit Aria2 Next proxy options. Applications should pass it when they own proxy configuration.

## BitTorrent Metadata

During magnet metadata download, `bittorrent.info` is omitted until stable torrent metadata exists.

Clients should read `bittorrent.metadata.state` and `bittorrent.metadata.hasMetadata`. The state is `downloading` while libtorrent is still fetching metadata or `hasMetadata` is false. The state is `ready` after metadata exists.

If a magnet `dn` display name is available during metadata download, Aria2 Next exposes it as `bittorrent.metadata.displayName`. After metadata is ready, `bittorrent.info.name` is the authoritative torrent name.

Clients should not treat an empty `bittorrent.info` object as valid metadata.

## ED2K Progress

ED2K low-level completed length can represent verified progress. `ed2k.visibleCompletedLength` exposes stable visible progress across active, waiting, paused, and stopped states when in-flight progress exists.

Clients should prefer `ed2k.visibleCompletedLength` for ED2K progress display when the field is present. HTTP, FTP, and BitTorrent progress semantics remain protocol-specific.

## HTTP Transfers

Segmented HTTP transfers validate Range responses before writing body data. Valid ranged responses must use `206 Partial Content`, a matching `Content-Range`, and identity encoding.

Servers that ignore Range with `200 OK` can be downgraded to single full-body transfer. Clients should not assume the requested connection count remains fixed for the lifetime of a task, because Aria2 Next may adapt HTTP stream concurrency for reliability.

## Checksums

Whole-file checksums from input files or options are verified after transfer completion. Clients should wait for the final task result instead of assuming a task is complete as soon as network bytes reach total length.

## Session and Control Files

Session files and `.aria2` control files are owned by the engine. Applications can pass `--save-session`, `--input-file`, `--save-session-interval`, and related options, but should not edit control files directly while the engine owns them.

## Recommended Sidecar Inputs

Parent applications should pass explicit RPC binding options, an RPC secret, a clear proxy mode, a controlled session path, and debug log paths that users can export for diagnostics.

Integration bug reports should include the engine command line, JSON-RPC request and response, Aria2 Next version, platform, parent application version, and debug log. Redact secrets and private URLs.
