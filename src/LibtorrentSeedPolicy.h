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
#ifndef D_LIBTORRENT_SEED_POLICY_H
#define D_LIBTORRENT_SEED_POLICY_H

#include <chrono>
#include <cstdint>

namespace aria2 {

class Option;

bool hasLibtorrentSeedLimit(const Option* option);

bool shouldStopLibtorrentSeeding(const Option* option, int64_t completedLength,
                                 int64_t uploadLength,
                                 std::chrono::seconds seedingDuration);

} // namespace aria2

#endif // D_LIBTORRENT_SEED_POLICY_H
