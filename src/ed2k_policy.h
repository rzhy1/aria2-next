/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2026 aria2-next contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/* copyright --> */
#ifndef D_ED2K_POLICY_H
#define D_ED2K_POLICY_H

#include "common.h"

#include <memory>
#include <vector>

#include "Command.h"
#include "ed2k_peer.h"

namespace aria2 {

class Segment;
class SegmentMan;

namespace ed2k {

int sourcePriority(uint32_t sourceFlags);
PeerState* selectConnectPeer(std::vector<PeerState>& peers, int64_t now);
std::vector<std::shared_ptr<Segment>>
selectRequestSegments(SegmentMan* segmentMan, cuid_t cuid,
                      const std::vector<bool>& peerAvailability,
                      size_t maxSegments);

} // namespace ed2k

} // namespace aria2

#endif // D_ED2K_POLICY_H
