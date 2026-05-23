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
#include "CurlDownloadCommand.h"

#include <algorithm>
#include <cstring>

#include "CurlSession.h"
#include "DiskAdaptor.h"
#include "DownloadContext.h"
#include "DownloadEngine.h"
#include "DownloadFailureException.h"
#include "FileEntry.h"
#include "LogFactory.h"
#include "Logger.h"
#include "PeerStat.h"
#include "PieceStorage.h"
#include "NullProgressInfoFile.h"
#include "Request.h"
#include "RequestGroup.h"
#include "Segment.h"
#include "SegmentMan.h"
#include "fmt.h"

namespace aria2 {

CurlDownloadCommand::CurlDownloadCommand(
    cuid_t cuid, const std::shared_ptr<Request>& req,
    const std::shared_ptr<FileEntry>& fileEntry, RequestGroup* requestGroup,
    DownloadEngine* e)
    : AbstractCommand(cuid, req, fileEntry, requestGroup, e, nullptr, nullptr),
      easy_(nullptr),
      session_(nullptr),
      initialized_(false),
      finished_(false),
      expectedLength_(0)
{
  std::memset(errorBuffer_, 0, sizeof(errorBuffer_));
  peerStat_ = req->initPeerStat();
  peerStat_->downloadStart();
  if (getSegmentMan()) {
    getSegmentMan()->registerPeerStat(peerStat_);
  }
}

CurlDownloadCommand::~CurlDownloadCommand()
{
  if (session_ && easy_) {
    session_->remove(easy_);
  }
  if (easy_) {
    curl_easy_cleanup(easy_);
  }
  peerStat_->downloadStop();
  if (getSegmentMan()) {
    getSegmentMan()->updateFastestPeerStat(peerStat_);
  }
}

bool CurlDownloadCommand::execute()
{
  if (!getPieceStorage()) {
    getRequestGroup()->preDownloadProcessing();
    getRequestGroup()->adjustFilename(
        std::make_shared<NullProgressInfoFile>());
    getRequestGroup()->initPieceStorage();
    getPieceStorage()->getDiskAdaptor()->initAndOpenFile();
    getSegmentMan()->getSegment(getCuid(), 1);
  }

  return AbstractCommand::execute();
}

bool CurlDownloadCommand::executeInternal()
{
  if (!initialized_) {
    initialize();
  }

  session_->perform();

  int queued = 0;
  while (auto msg = curl_multi_info_read(session_->multi(), &queued)) {
    if (msg->msg == CURLMSG_DONE && msg->easy_handle == easy_) {
      finish(msg->data.result);
      return true;
    }
  }

  addCommandSelf();
  getDownloadEngine()->scheduleRuntimeWake(std::chrono::milliseconds(10));
  return false;
}

void CurlDownloadCommand::initialize()
{
  easy_ = curl_easy_init();
  if (!easy_) {
    throw DOWNLOAD_FAILURE_EXCEPTION("Failed to initialize libcurl easy.");
  }

  session_ = &getDownloadEngine()->getCurlSession();

  expectedLength_ = getFileEntry()->getLength();
  if (getSegmentMan()) {
    getSegmentMan()->registerPeerStat(peerStat_);
  }

  curl_easy_setopt(easy_, CURLOPT_URL, getRequest()->getCurrentUri().c_str());
  curl_easy_setopt(easy_, CURLOPT_WRITEFUNCTION,
                   &CurlDownloadCommand::writeCallback);
  curl_easy_setopt(easy_, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(easy_, CURLOPT_ERRORBUFFER, errorBuffer_);
  curl_easy_setopt(easy_, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(easy_, CURLOPT_NOSIGNAL, 1L);
  curl_easy_setopt(easy_, CURLOPT_FAILONERROR, 0L);

  const auto& segments = getSegments();
  if (!segments.empty() && expectedLength_ > 0) {
    const auto& segment = segments.front();
    const auto begin = getFileEntry()->gtoloff(segment->getPositionToWrite());
    const auto end =
        getFileEntry()->gtoloff(segment->getPosition() + segment->getLength()) -
        1;
    curl_easy_setopt(easy_, CURLOPT_RANGE,
                     fmt("%lld-%lld", static_cast<long long>(begin),
                         static_cast<long long>(end))
                         .c_str());
  }

  session_->add(easy_, this);
  session_->perform();
  initialized_ = true;
}

void CurlDownloadCommand::finish(CURLcode result)
{
  finished_ = true;
  long status = 0;
  curl_easy_getinfo(easy_, CURLINFO_RESPONSE_CODE, &status);

  if (result != CURLE_OK) {
    throw DOWNLOAD_FAILURE_EXCEPTION(
        fmt("libcurl transfer failed: %s",
            errorBuffer_[0] ? errorBuffer_ : curl_easy_strerror(result)));
  }
  if (status >= 400) {
    throw DOWNLOAD_FAILURE_EXCEPTION(
        fmt("HTTP transfer failed with status %ld.", status));
  }

  completeCurrentSegment();
  if (getRequestGroup()->downloadFinished()) {
    getDownloadEngine()->setNoWait(true);
    getDownloadEngine()->setRefreshInterval(std::chrono::milliseconds(0));
    return;
  }

  getDownloadEngine()->addCommand(
      make_unique<CurlDownloadCommand>(getCuid(), getRequest(), getFileEntry(),
                                       getRequestGroup(), getDownloadEngine()));
}

void CurlDownloadCommand::completeCurrentSegment()
{
  const auto& segments = getSegments();
  if (segments.empty()) {
    return;
  }
  const auto& segment = segments.front();
  getPieceStorage()->flushWrDiskCacheEntry(false);
  getSegmentMan()->completeSegment(getCuid(), segment);
}

size_t CurlDownloadCommand::writeCallback(char* ptr, size_t size, size_t nmemb,
                                          void* userdata)
{
  auto self = static_cast<CurlDownloadCommand*>(userdata);
  return self->writeData(reinterpret_cast<const unsigned char*>(ptr),
                         size * nmemb);
}

size_t CurlDownloadCommand::writeData(const unsigned char* data, size_t length)
{
  if (length == 0) {
    return 0;
  }

  auto remaining = length;
  auto cursor = data;
  while (remaining > 0) {
    const auto& segments = getSegments();
    if (segments.empty()) {
      return length - remaining;
    }
    const auto& segment = segments.front();
    const auto capacity =
        segment->getLength() > 0
            ? static_cast<size_t>(segment->getLength() -
                                  segment->getWrittenLength())
            : remaining;
    const auto writeLength = std::min(remaining, capacity);
    getPieceStorage()->getDiskAdaptor()->writeData(
        cursor, writeLength, segment->getPositionToWrite());
    segment->updateWrittenLength(writeLength);
    peerStat_->updateDownload(writeLength);
    getDownloadContext()->updateDownload(writeLength);
    cursor += writeLength;
    remaining -= writeLength;
    if (writeLength == 0) {
      break;
    }
  }

  return length - remaining;
}

} // namespace aria2
