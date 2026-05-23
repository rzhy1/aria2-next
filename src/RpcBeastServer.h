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
#ifndef D_RPC_BEAST_SERVER_H
#define D_RPC_BEAST_SERVER_H

#include "common.h"

#include <memory>

#include <boost/asio/ip/tcp.hpp>

namespace aria2 {

class DownloadEngine;

class RpcBeastServer : public std::enable_shared_from_this<RpcBeastServer> {
public:
  RpcBeastServer(DownloadEngine* engine, int family);
  ~RpcBeastServer();

  bool bindPort(uint16_t port);
  void stop();

private:
  void accept();

  DownloadEngine* engine_;
  int family_;
  boost::asio::ip::tcp::acceptor acceptor_;
};

} // namespace aria2

#endif // D_RPC_BEAST_SERVER_H
