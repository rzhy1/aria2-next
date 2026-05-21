# ED2K Download Hardening Progress

This file is the compact chronological evidence trail for ED2K download
hardening. Keep entries short and checkpoint-sized. Do not paste raw logs,
packet captures, generated reports, local caches, public-network scratch data,
or conversation text.

## Current State

The completed `docs/maintenance/ed2k-refactor` tracker is the protocol-aligned
baseline. Public testing and comparison with aMule indicate the next reliability
gap is runtime behavior: source lifecycle, long-running discovery, queued-peer
reask, LowID handling, stable identity, transfer pacing, and cooperative upload
state.

## Entries

### AR00 - Tracker Activation and Baseline Audit

Changed: Activated the ED2K download hardening tracker as the next workstream
after the completed `ed2k-refactor` protocol baseline. The maintenance root
index points to `ed2k-download-hardening`, the tracker read path stays narrow,
and AR00 records baseline runtime findings without rewriting historical ED2K
completion records.

Reference evidence: Focused aMule inspection covered
`CDownloadQueue::Process`, `ProcessLocalRequests`, `SendNextUDPPacket`,
`DoKademliaFileRequest`, `CPartFile::Process`,
`CUpDownClient::IsSourceRequestAllowed`, `UDPReaskACK`, `UDPReaskFNF`,
`UDPReaskForDownload`, `CDeadSourceList`, `CClientCredits`, `CUploadQueue`,
`CSharedFileList`, and Kad search/firewall paths. The important aMule runtime
model is durable source discovery, source quality, dead-source expiry, queued
peer reask, LowID handling, transfer pacing, upload cooperation, and stable
identity.

Current-code evidence: Focused aria2-next inspection covered `Ed2kAttribute`,
`Ed2kCommand`, `Ed2kKadCommand`, `ed2k_policy`, `ed2k_peer`,
`Ed2kUploadQueue`, `Ed2kSharedResponder`, `SessionSerializer`,
`RpcMethodImpl`, and the existing ED2K tests. The current code already has
native protocol surfaces, but later checkpoints must harden identity,
source-policy state, server/Kad cadence, peer lifecycle, queued-peer reask,
LowID boundaries, transfer pacing, and status truth.

Verified: `git diff --check docs/maintenance` passed. The CSV parser check
passed for all `docs/maintenance/ed2k-download-hardening` CSV files.

Remaining: Start AR10 stable ED2K identity and runtime metadata.
