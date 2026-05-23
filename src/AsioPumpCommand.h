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
#ifndef D_ASIO_PUMP_COMMAND_H
#define D_ASIO_PUMP_COMMAND_H

#include "Command.h"

namespace aria2 {

class DownloadEngine;

class AsioPumpCommand : public Command {
public:
  AsioPumpCommand(cuid_t cuid, DownloadEngine* engine);

  bool execute() CXX11_OVERRIDE;

private:
  DownloadEngine* engine_;
};

} // namespace aria2

#endif // D_ASIO_PUMP_COMMAND_H
