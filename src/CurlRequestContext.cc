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
#include "CurlRequestContext.h"

#include "util.h"

namespace aria2 {
namespace {

std::string fieldName(const std::string& header)
{
  auto colon = header.find(':');
  if (colon == std::string::npos) {
    return "";
  }
  auto name = util::strip(header.substr(0, colon));
  util::lowercase(name);
  return name;
}

std::string fieldValue(const std::string& header)
{
  auto colon = header.find(':');
  if (colon == std::string::npos) {
    return "";
  }
  return util::strip(header.substr(colon + 1));
}

} // namespace

CurlRequestContext buildCurlRequestContext(
    const CurlRequestContextInput& input)
{
  CurlRequestContext context;
  context.userAgent = input.userAgent;
  if (!input.referer.empty()) {
    context.hasReferer = true;
    context.referer = input.referer;
  }

  for (const auto& header : input.headers) {
    const auto name = fieldName(header);
    if (name == "user-agent") {
      context.userAgent = fieldValue(header);
      continue;
    }
    if (name == "referer" || name == "referrer") {
      context.hasReferer = true;
      context.referer = fieldValue(header);
      continue;
    }
    if (name == "cookie") {
      context.hasCookie = true;
      context.cookie = fieldValue(header);
      continue;
    }
    if (name == "accept-encoding") {
      context.hasAcceptEncoding = true;
      context.acceptEncoding = fieldValue(header);
      continue;
    }
    context.headers.push_back(header);
  }

  if (!context.hasAcceptEncoding) {
    context.hasAcceptEncoding = true;
    context.acceptEncoding = input.httpAcceptGzip ? "" : "identity";
  }

  return context;
}

} // namespace aria2
