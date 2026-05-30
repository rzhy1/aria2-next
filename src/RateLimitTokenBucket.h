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
#ifndef D_RATE_LIMIT_TOKEN_BUCKET_H
#define D_RATE_LIMIT_TOKEN_BUCKET_H

#include "common.h"

#include <stdint.h>

#include "TimerA2.h"

namespace aria2 {

class RateLimitTokenBucket {
public:
  size_t consume(int64_t limit, size_t requested);

  void reset();

private:
  Timer updatedAt_ = Timer::zero();
  int64_t tokens_ = 0;
  bool initialized_ = false;
};

} // namespace aria2

#endif // D_RATE_LIMIT_TOKEN_BUCKET_H
