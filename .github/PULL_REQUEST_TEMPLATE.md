<!--
Read docs/CONTRIBUTING.md before submitting.

PR titles must use Conventional Commits:
  fix(http): retry transient segmented transfer failures
  feat(rpc): expose ED2K visible progress
  docs: tighten troubleshooting guidance
  ci(release): add Windows debug artifacts
-->

## Description

<!-- What changed, why, and which issue it closes. -->

## Affected Surface

- [ ] CLI options or configuration
- [ ] JSON-RPC behavior
- [ ] HTTP / HTTPS / FTP / SFTP transfers
- [ ] BitTorrent / magnet / libtorrent
- [ ] ED2K
- [ ] Session, input file, or control file handling
- [ ] Checksum or integrity verification
- [ ] Build, dependency, or release packaging
- [ ] Documentation only

## Compatibility

<!-- Describe aria2-compatible behavior changes, Aria2 Next extensions, or Motrix Next integration impact. -->

## Verification

<!-- Paste exact commands. Do not write only "it builds". -->

```bash
cmake --preset default
cmake --build --preset default
ctest --preset default
build/default/aria2-next --version
```

## Checklist

- [ ] I read `docs/CONTRIBUTING.md`.
- [ ] The PR addresses one concern.
- [ ] User-facing behavior changes are documented.
- [ ] New or changed CLI/RPC behavior has focused tests.
- [ ] Packaging changes were verified with the affected release path or script checks.
- [ ] Dependency version or hash changes update `packaging/dependencies.env`.
- [ ] No generated build output, binaries, logs, dumps, or local caches are committed.
- [ ] Commits follow Conventional Commits.

## AI Usage

- [ ] No AI tools were used.
- [ ] AI tools assisted with drafting, refactoring, or boilerplate. I reviewed and understand every line.
- [ ] Substantial portions were AI-generated. I reviewed, tested, and can explain every change.

Tool used, if any:

## Release Notes

<!-- One user-facing sentence, or "none" if not user-facing. -->
