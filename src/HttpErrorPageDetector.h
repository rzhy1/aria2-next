/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2026 AnInsomniacy
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/* copyright --> */
#ifndef D_HTTP_ERROR_PAGE_DETECTOR_H
#define D_HTTP_ERROR_PAGE_DETECTOR_H

#include <string>

namespace aria2 {

struct HttpErrorPageDecision {
  bool reject = false;
  std::string reason;
};

HttpErrorPageDecision detectHttpErrorPage(const std::string& targetPath,
                                          const std::string& contentType,
                                          const std::string& bodyPrefix);

} // namespace aria2

#endif // D_HTTP_ERROR_PAGE_DETECTOR_H
