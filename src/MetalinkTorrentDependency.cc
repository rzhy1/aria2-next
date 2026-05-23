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
#include "MetalinkTorrentDependency.h"

#include <algorithm>

#include "DownloadContext.h"
#include "DiskAdaptor.h"
#include "File.h"
#include "FileEntry.h"
#include "LogFactory.h"
#include "Logger.h"
#include "PieceStorage.h"
#include "RecoverableException.h"
#include "RequestGroup.h"
#include "SimpleRandomizer.h"
#include "bittorrent_helper.h"
#include "fmt.h"

namespace aria2 {

MetalinkTorrentDependency::MetalinkTorrentDependency(
    RequestGroup* dependant, const std::shared_ptr<RequestGroup>& dependee)
    : dependant_(dependant), dependee_(dependee)
{
}

MetalinkTorrentDependency::~MetalinkTorrentDependency() = default;

namespace {
void copyValues(const std::shared_ptr<FileEntry>& d,
                const std::shared_ptr<FileEntry>& s)
{
  d->setRequested(true);
  d->setPath(s->getPath());
  d->addUris(std::begin(s->getRemainingUris()),
             std::end(s->getRemainingUris()));
  d->setMaxConnectionPerServer(s->getMaxConnectionPerServer());
  d->setUniqueProtocol(s->isUniqueProtocol());
}

struct EntryCmp {
  bool operator()(const std::shared_ptr<FileEntry>& lhs,
                  const std::shared_ptr<FileEntry>& rhs) const
  {
    return lhs->getOriginalName() < rhs->getOriginalName();
  }
};
} // namespace

bool MetalinkTorrentDependency::resolve()
{
  if (!dependee_) {
    return true;
  }
  if (dependee_->getNumCommand() == 0 && dependee_->downloadFinished()) {
    std::shared_ptr<RequestGroup> dependee = dependee_;
    dependee_.reset();
    const auto& dependantContext = dependant_->getDownloadContext();
    const auto& hashType = dependantContext->getHashType();
    const auto& digest = dependantContext->getDigest();
    auto context = std::make_shared<DownloadContext>();
    try {
      std::shared_ptr<DiskAdaptor> diskAdaptor =
          dependee->getPieceStorage()->getDiskAdaptor();
      diskAdaptor->openExistingFile();
      bittorrent::loadFromMemory(
          util::toString(diskAdaptor), context, dependant_->getOption(),
          File(dependee->getFirstFilePath()).getBasename());
      bittorrent::adjustAnnounceUri(bittorrent::getTorrentAttrs(context),
                                    dependant_->getOption());
      const std::vector<std::shared_ptr<FileEntry>>& fileEntries =
          context->getFileEntries();
      for (auto& fe : fileEntries) {
        auto& uri = fe->getRemainingUris();
        std::shuffle(std::begin(uri), std::end(uri),
                     *SimpleRandomizer::getInstance());
      }
      const std::vector<std::shared_ptr<FileEntry>>& dependantFileEntries =
          dependant_->getDownloadContext()->getFileEntries();
      if (fileEntries.size() == 1 && dependantFileEntries.size() == 1 &&
          dependantFileEntries[0]->getOriginalName().empty()) {
        copyValues(fileEntries[0], dependantFileEntries[0]);
      }
      else {
        std::vector<std::shared_ptr<FileEntry>> destFiles;
        destFiles.reserve(fileEntries.size());
        for (auto& e : fileEntries) {
          e->setRequested(false);
          destFiles.push_back(e);
        }
        std::sort(std::begin(destFiles), std::end(destFiles), EntryCmp());
        for (const auto& e : dependantFileEntries) {
          const auto d = std::lower_bound(std::begin(destFiles),
                                          std::end(destFiles), e, EntryCmp());
          if (d == std::end(destFiles) ||
              (*d)->getOriginalName() != e->getOriginalName()) {
            throw DL_ABORT_EX(fmt("No entry %s in torrent file",
                                  e->getOriginalName().c_str()));
          }
          copyValues(*d, e);
        }
      }
    }
    catch (RecoverableException& e) {
      A2_LOG_ERROR_EX(EX_EXCEPTION_CAUGHT, e);
      A2_LOG_INFO(fmt("Metalink BitTorrent dependency for GID#%s failed.",
                      GroupId::toHex(dependant_->getGID()).c_str()));
      return true;
    }
    A2_LOG_INFO(fmt("Dependency resolved for GID#%s",
                    GroupId::toHex(dependant_->getGID()).c_str()));
    if (!hashType.empty() && !digest.empty()) {
      context->setDigest(hashType, digest);
    }
    dependant_->setDownloadContext(context);
    return true;
  }
  else if (dependee_->getNumCommand() == 0) {
    dependee_.reset();
    A2_LOG_INFO(fmt("Metalink BitTorrent dependency for GID#%s failed.",
                    GroupId::toHex(dependant_->getGID()).c_str()));
    return true;
  }
  else {
    return false;
  }
}

} // namespace aria2
