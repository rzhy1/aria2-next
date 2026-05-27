# Documentation

This directory contains the maintained documentation for aria2-next.

## Start Here

| Document | Audience | Purpose |
| --- | --- | --- |
| [`../README.md`](../README.md) | Users and integrators | Project scope, downloads, quick start, build command, and repository layout |
| [`CONTRIBUTING.md`](CONTRIBUTING.md) | Contributors | Local setup, code rules, test expectations, dependency policy, and PR requirements |
| [`TROUBLESHOOTING.md`](TROUBLESHOOTING.md) | Users and maintainers | Common failures, report boundaries, required logs, and safe redaction guidance |
| [`INTEGRATION.md`](INTEGRATION.md) | App authors and RPC clients | Stable CLI, session, and JSON-RPC contracts for parent applications |
| [`RELEASE.md`](RELEASE.md) | Users and maintainers | Release assets, checksum verification, debug artifacts, release flow, and recovery rules |
| [`SECURITY.md`](SECURITY.md) | Security reporters | Vulnerability scope, RPC exposure notes, and sensitive-report handling |
| [`PRIVACY.md`](PRIVACY.md) | Users and integrators | Runtime network behavior, local files, logs, and parent-application boundaries |
| [`CODE_OF_CONDUCT.md`](CODE_OF_CONDUCT.md) | Everyone | Conduct rules for project spaces |

## Reference Material

| Path | Purpose |
| --- | --- |
| [`manual/`](manual/) | Sphinx manual sources for CLI, options, RPC, and technical notes |
| [`completion/`](completion/) | Bash completion source and generation tooling |
| [`maintenance/`](maintenance/) | Durable modernization, migration, and audit records |
| [`licenses/`](licenses/) | Preserved license exception text and license notes |
| [`media/`](media/) | Images used by project documentation |

GitHub issue forms and the pull request template live under [`../.github/`](../.github/). They are intentionally short intake forms, not separate documentation.

Temporary logs, packet captures, workflow scratch files, local release staging folders, generated reports, and downloaded test files do not belong in this directory.
