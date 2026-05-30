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
#ifndef D_ED2K_OUTBOUND_PACKET_H
#define D_ED2K_OUTBOUND_PACKET_H

#include "common.h"

#include <string>

namespace aria2 {

namespace ed2k {

struct OutboundPacket {
  std::string data;
  bool fileData = false;
  bool encrypted = false;
};

} // namespace ed2k

} // namespace aria2

#endif // D_ED2K_OUTBOUND_PACKET_H
