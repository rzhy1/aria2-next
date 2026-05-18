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
#include "Ed2kSharedStore.h"

#include <algorithm>

#include "DownloadResult.h"
#include "Ed2kAttribute.h"
#include "File.h"
#include "FileEntry.h"
#include "ed2k_hash.h"
#include "error_code.h"

namespace aria2 {

namespace ed2k {

namespace {

const Ed2kAttribute* findEd2kAttrs(const DownloadResult& result)
{
  for (const auto& attrs : result.attrs) {
    auto ed2kAttrs = dynamic_cast<const Ed2kAttribute*>(attrs.get());
    if (ed2kAttrs) {
      return ed2kAttrs;
    }
  }
  return nullptr;
}

} // namespace

bool isValidSharedFile(const SharedFile& file)
{
  if (file.hash.size() != HASH_LENGTH || file.path.empty() ||
      file.name.empty() || file.size <= 0) {
    return false;
  }
  File diskFile(file.path);
  return diskFile.isFile() && diskFile.size() == file.size;
}

bool SharedStore::addOrReplace(SharedFile file)
{
  if (!isValidSharedFile(file)) {
    return false;
  }
  auto itr = std::find_if(std::begin(files_), std::end(files_),
                          [&](const SharedFile& item) {
                            return item.hash == file.hash;
                          });
  if (itr == std::end(files_)) {
    files_.push_back(std::move(file));
  }
  else {
    *itr = std::move(file);
  }
  return true;
}

bool SharedStore::addCompletedDownload(const DownloadResult& result,
                                       int64_t now)
{
  if (result.result != error_code::FINISHED || result.fileEntries.size() != 1 ||
      result.inMemoryDownload) {
    return false;
  }
  auto attrs = findEd2kAttrs(result);
  if (!attrs || attrs->link.hash.size() != HASH_LENGTH ||
      attrs->link.size != result.totalLength || attrs->link.size <= 0) {
    return false;
  }
  const auto& fileEntry = result.fileEntries[0];
  if (!fileEntry || fileEntry->getLength() != attrs->link.size) {
    return false;
  }

  SharedFile file;
  file.hash = attrs->link.hash;
  file.aichRootHash = attrs->aichRootHash;
  file.pieceHashes = attrs->pieceHashes;
  file.path = fileEntry->getPath();
  file.name = attrs->link.name.empty() ? fileEntry->getBasename()
                                       : attrs->link.name;
  file.size = attrs->link.size;
  file.lastHashTime = now;
  file.origin = SharedOrigin::COMPLETED_DOWNLOAD;
  file.completed = true;
  return addOrReplace(std::move(file));
}

const SharedFile* SharedStore::findByHash(const std::string& hash) const
{
  auto itr = std::find_if(std::begin(files_), std::end(files_),
                          [&](const SharedFile& item) {
                            return item.hash == hash;
                          });
  return itr == std::end(files_) ? nullptr : &*itr;
}

} // namespace ed2k

} // namespace aria2
