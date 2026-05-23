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
#ifndef D_METALINK_TORRENT_DEPENDENCY_H
#define D_METALINK_TORRENT_DEPENDENCY_H

#include "Dependency.h"

#include <memory>

namespace aria2 {

class RequestGroup;

class MetalinkTorrentDependency : public Dependency {
private:
  RequestGroup* dependant_;
  std::shared_ptr<RequestGroup> dependee_;

public:
  MetalinkTorrentDependency(RequestGroup* dependant,
                            const std::shared_ptr<RequestGroup>& dependee);

  virtual ~MetalinkTorrentDependency();

  virtual bool resolve() CXX11_OVERRIDE;
};

} // namespace aria2

#endif // D_METALINK_TORRENT_DEPENDENCY_H
