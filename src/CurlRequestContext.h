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
#ifndef D_CURL_REQUEST_CONTEXT_H
#define D_CURL_REQUEST_CONTEXT_H

#include <string>
#include <vector>

namespace aria2 {

struct CurlRequestContextInput {
  std::string userAgent;
  std::string referer;
  bool httpAcceptGzip = false;
  std::vector<std::string> headers;
};

struct CurlRequestContext {
  std::string userAgent;
  bool hasReferer = false;
  std::string referer;
  bool hasCookie = false;
  std::string cookie;
  bool hasAcceptEncoding = false;
  std::string acceptEncoding;
  std::vector<std::string> headers;
};

CurlRequestContext buildCurlRequestContext(
    const CurlRequestContextInput& input);

} // namespace aria2

#endif // D_CURL_REQUEST_CONTEXT_H
