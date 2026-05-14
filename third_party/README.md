# Third-Party Source

This directory contains source code that is bundled into aria2-next builds.

## wslay

`third_party/wslay` provides the WebSocket implementation used by aria2 JSON-RPC
over WebSocket support. The bundled source is retained because wslay 1.1.1 is
still the current upstream release and the existing autotools build integrates it
as a subproject.

aria2-next does not apply aria2 formatting rules to files under `third_party/`.

Future work may add `--with-system-wslay` support so distributions can link
against a system-provided wslay package. That is intentionally not part of the
current structure cleanup because the goal is to preserve existing build
behavior while making the ownership boundary explicit.
