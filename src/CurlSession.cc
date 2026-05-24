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
#include "CurlSession.h"

#include "DownloadFailureException.h"
#include "DownloadEngine.h"
#include "EventPoll.h"
#include "fmt.h"

namespace aria2 {

CurlSession::CurlSession(DownloadEngine* engine)
    : engine_(engine), multi_(curl_multi_init()), runningHandles_(0)
{
  if (!multi_) {
    throw DOWNLOAD_FAILURE_EXCEPTION("Failed to initialize libcurl multi.");
  }
  curl_multi_setopt(multi_, CURLMOPT_SOCKETFUNCTION,
                    &CurlSession::socketCallback);
  curl_multi_setopt(multi_, CURLMOPT_SOCKETDATA, this);
  curl_multi_setopt(multi_, CURLMOPT_TIMERFUNCTION,
                    &CurlSession::timerCallback);
  curl_multi_setopt(multi_, CURLMOPT_TIMERDATA, this);
}

CurlSession::~CurlSession()
{
  for (const auto& entry : sockets_) {
    engine_->deleteRawSocketCheck(entry.first, entry.second.first,
                                  entry.second.second);
  }
  if (multi_) {
    curl_multi_cleanup(multi_);
  }
}

void CurlSession::perform()
{
  CURLMcode rc = curl_multi_perform(multi_, &runningHandles_);
  if (rc != CURLM_OK) {
    throw DOWNLOAD_FAILURE_EXCEPTION(
        fmt("libcurl multi perform failed: %s", curl_multi_strerror(rc)));
  }
  drainMessages();
}

void CurlSession::add(CURL* easy, Command* command)
{
  curl_easy_setopt(easy, CURLOPT_PRIVATE, command);
  CURLMcode rc = curl_multi_add_handle(multi_, easy);
  if (rc != CURLM_OK) {
    throw DOWNLOAD_FAILURE_EXCEPTION(
        fmt("Failed to add libcurl transfer: %s", curl_multi_strerror(rc)));
  }
}

void CurlSession::remove(CURL* easy)
{
  if (!easy) {
    return;
  }
  doneResults_.erase(easy);
  curl_multi_remove_handle(multi_, easy);
}

bool CurlSession::takeDoneResult(CURL* easy, CURLcode& result)
{
  auto i = doneResults_.find(easy);
  if (i == doneResults_.end()) {
    return false;
  }
  result = i->second;
  doneResults_.erase(i);
  return true;
}

int CurlSession::socketCallback(CURL* easy, curl_socket_t socket, int action,
                                void* userdata, void* socketData)
{
  auto self = static_cast<CurlSession*>(userdata);
  Command* command = nullptr;
  curl_easy_getinfo(easy, CURLINFO_PRIVATE, &command);
  if (!command) {
    return 0;
  }

  if (action == CURL_POLL_REMOVE) {
    self->clearSocket(socket);
    return 0;
  }

  self->clearSocket(socket);
  self->addSocket(socket, command, action);
  return 0;
}

int CurlSession::timerCallback(CURLM* multi, long timeoutMs, void* userdata)
{
  auto self = static_cast<CurlSession*>(userdata);
  if (timeoutMs < 0) {
    return 0;
  }
  self->engine_->scheduleRuntimeWake(std::chrono::milliseconds(timeoutMs));
  return 0;
}

bool CurlSession::addSocket(curl_socket_t socket, Command* command, int action)
{
  int events = 0;
  if (action == CURL_POLL_IN || action == CURL_POLL_INOUT) {
    events |= EventPoll::EVENT_READ;
  }
  if (action == CURL_POLL_OUT || action == CURL_POLL_INOUT) {
    events |= EventPoll::EVENT_WRITE;
  }
  if (events == 0) {
    return true;
  }
  sockets_[socket] = std::make_pair(command, events);
  return engine_->addRawSocketCheck(socket, command, events);
}

bool CurlSession::deleteSocket(curl_socket_t socket, Command* command,
                               int events)
{
  return engine_->deleteRawSocketCheck(socket, command, events);
}

void CurlSession::clearSocket(curl_socket_t socket)
{
  auto i = sockets_.find(socket);
  if (i == sockets_.end()) {
    return;
  }
  deleteSocket(socket, i->second.first, i->second.second);
  sockets_.erase(i);
}

void CurlSession::drainMessages()
{
  int queued = 0;
  while (auto msg = curl_multi_info_read(multi_, &queued)) {
    if (msg->msg == CURLMSG_DONE) {
      doneResults_[msg->easy_handle] = msg->data.result;
    }
  }
}

} // namespace aria2
