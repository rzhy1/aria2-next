# Libtorrent BitTorrent Replacement Progress

This file is the compact chronological evidence trail for the libtorrent
BitTorrent replacement. Keep entries checkpoint-sized. Do not record raw logs,
packet captures, generated reports, local public-network data, or conversation
text.

Use this format:

```text
YYYY-MM-DD BTM-XXX status
Changed: concise tracker, code, or behavior summary.
Verified: exact final command and result, or documentation-only reason.
Remaining: next concrete gap.
Blocked: none, or exact blocker.
```

## Log

2026-05-23 BTM-001 active
Changed: Created the libtorrent BitTorrent replacement tracker with continuous
checkpoint numbering, a no-fallback migration contract, capability decisions,
focused test policy, native BitTorrent cleanup rules, and a separate
non-BitTorrent stall-fix checkpoint.
Verified: Documentation setup pending local format checks.
Remaining: Finish BTM-001 documentation verification, then start BTM-002
dependency and build integration.
Blocked: none.
