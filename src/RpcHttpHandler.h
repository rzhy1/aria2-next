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
#ifndef D_RPC_HTTP_HANDLER_H
#define D_RPC_HTTP_HANDLER_H

#include "common.h"

#include <map>
#include <string>

namespace aria2 {

class DownloadEngine;

struct RpcHttpRequest {
  std::string method;
  std::string target;
  std::map<std::string, std::string> headers;
  std::string body;
  bool acceptsGzip = false;
};

struct RpcHttpResponse {
  int status = 200;
  std::map<std::string, std::string> headers;
  std::string body;
  std::string contentType;
  bool gzip = false;
  bool closeConnection = false;
  bool delayAfterWrite = false;
};

class RpcHttpHandler {
public:
  explicit RpcHttpHandler(DownloadEngine* engine);

  RpcHttpResponse handle(const RpcHttpRequest& req) const;

private:
  DownloadEngine* engine_;
};

} // namespace aria2

#endif // D_RPC_HTTP_HANDLER_H
