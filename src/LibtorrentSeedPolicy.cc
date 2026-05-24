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
#include "LibtorrentSeedPolicy.h"

#include "Option.h"
#include "prefs.h"

namespace aria2 {

bool hasLibtorrentSeedLimit(const Option* option)
{
  return option->defined(PREF_SEED_TIME) ||
         option->getAsDouble(PREF_SEED_RATIO) > 0.0;
}

bool shouldStopLibtorrentSeeding(const Option* option, int64_t completedLength,
                                 int64_t uploadLength,
                                 std::chrono::seconds seedingDuration)
{
  return hasLibtorrentSeedLimit(option) &&
         shouldStopLibtorrentSharing(option, completedLength, uploadLength,
                                     seedingDuration);
}

bool shouldStopLibtorrentSharing(const Option* option, int64_t completedLength,
                                 int64_t uploadLength,
                                 std::chrono::seconds sharingDuration)
{
  if (option->defined(PREF_SEED_TIME)) {
    auto seedTime = std::chrono::seconds(
        static_cast<int>(option->getAsDouble(PREF_SEED_TIME) * 60));
    if (sharingDuration >= seedTime) {
      return true;
    }
  }

  auto ratio = option->getAsDouble(PREF_SEED_RATIO);
  if (ratio <= 0.0) {
    return false;
  }
  if (completedLength == 0) {
    return true;
  }
  return ratio <= 1.0 * uploadLength / completedLength;
}

} // namespace aria2
