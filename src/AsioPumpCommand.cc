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
#include "AsioPumpCommand.h"

#include "DownloadEngine.h"
#include "RequestGroupMan.h"

namespace aria2 {

AsioPumpCommand::AsioPumpCommand(cuid_t cuid, DownloadEngine* engine)
    : Command(cuid), engine_(engine)
{
  setStatus(Command::STATUS_ONESHOT_REALTIME);
}

bool AsioPumpCommand::execute()
{
  if (engine_->isHaltRequested() ||
      engine_->getRequestGroupMan()->downloadFinished()) {
    return true;
  }
  engine_->scheduleRuntimeWake(std::chrono::milliseconds(10));
  setStatus(Command::STATUS_ONESHOT_REALTIME);
  engine_->addCommand(std::unique_ptr<Command>(this));
  return false;
}

} // namespace aria2
