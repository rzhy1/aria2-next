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
#include "RateLimitTokenBucket.h"

#include <algorithm>

#include "wallclock.h"

namespace aria2 {

size_t RateLimitTokenBucket::consume(int64_t limit, size_t requested)
{
  if (limit <= 0 || requested == 0) {
    return requested;
  }

  if (!initialized_) {
    updatedAt_ = global::wallclock();
    tokens_ = std::max<int64_t>(1, limit / 2);
    initialized_ = true;
  }
  else {
    const auto elapsed =
        updatedAt_.difference(global::wallclock()).count();
    if (elapsed > 0) {
      tokens_ = std::min<int64_t>(
          limit, tokens_ + (limit * static_cast<int64_t>(elapsed)) / 1000);
      updatedAt_ = global::wallclock();
    }
  }

  const auto allowed =
      std::min<int64_t>(static_cast<int64_t>(requested), tokens_);
  if (allowed <= 0) {
    return 0;
  }
  tokens_ -= allowed;
  return static_cast<size_t>(allowed);
}

void RateLimitTokenBucket::reset()
{
  updatedAt_ = Timer::zero();
  tokens_ = 0;
  initialized_ = false;
}

} // namespace aria2
