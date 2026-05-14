# Packaging

This directory owns release packaging, cross-compilation helpers, Docker build contexts, platform package resources, and release dependency metadata.

`dependencies.env` is the authoritative dependency version source for maintained release automation.

## Layout

| Path | Purpose |
| --- | --- |
| `notes/` | Platform notes copied into binary packages |
| `docker/` | Dockerfiles for reproducible cross-platform build images |
| `macos/` | macOS package resources |
| `scripts/` | Release packaging helpers |
| `dependencies.env` | Maintained dependency baseline |

Supported packaging paths build this repository checkout through CMake. Third-party dependencies may use their own upstream build systems while they are being built as release dependencies.

Binary packages should include `README.md`, license files, and the relevant platform note from `notes/`. Release history belongs in git tags and GitHub Releases.
