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
#ifndef D_CURL_DOWNLOAD_COMMAND_H
#define D_CURL_DOWNLOAD_COMMAND_H

#include "AbstractCommand.h"

#include <curl/curl.h>

namespace aria2 {

class CurlSession;
class PeerStat;

class CurlDownloadCommand : public AbstractCommand {
public:
  CurlDownloadCommand(cuid_t cuid, const std::shared_ptr<Request>& req,
                      const std::shared_ptr<FileEntry>& fileEntry,
                      RequestGroup* requestGroup, DownloadEngine* e);
  ~CurlDownloadCommand() CXX11_OVERRIDE;

private:
  bool execute() CXX11_OVERRIDE;

  bool executeInternal() CXX11_OVERRIDE;
  bool noCheck() const CXX11_OVERRIDE { return true; }

  void initialize();
  void finish(CURLcode result);
  void completeCurrentSegment();

  static size_t writeCallback(char* ptr, size_t size, size_t nmemb,
                              void* userdata);

  size_t writeData(const unsigned char* data, size_t length);

  CURL* easy_;
  CurlSession* session_;
  std::shared_ptr<PeerStat> peerStat_;
  bool initialized_;
  bool finished_;
  int64_t expectedLength_;
  char errorBuffer_[CURL_ERROR_SIZE];
};

} // namespace aria2

#endif // D_CURL_DOWNLOAD_COMMAND_H
