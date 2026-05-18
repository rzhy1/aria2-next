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
#include "Ed2kSharedResponder.h"

#include <vector>

#include "Ed2kSharedFile.h"
#include "Ed2kSharedStore.h"
#include "ed2k_aich.h"
#include "ed2k_constants.h"
#include "ed2k_hash.h"
#include "ed2k_packet.h"
#include "ed2k_peer.h"

namespace aria2 {

namespace ed2k {

SharedResponder::SharedResponder(SharedStore* store,
                                 std::deque<std::string>& outbox)
    : store_(store), outbox_(&outbox)
{
}

const SharedFile* SharedResponder::findFile(const std::string& hash) const
{
  return store_ ? store_->findByHash(hash) : nullptr;
}

bool SharedResponder::hasFile(const std::string& hash) const
{
  return findFile(hash) != nullptr;
}

void SharedResponder::queuePacket(uint8_t protocol, uint8_t opcode,
                                  const std::string& payload)
{
  outbox_->push_back(createPacket(protocol, opcode, payload));
}

void SharedResponder::queueNoFile(const std::string& fileHash)
{
  if (fileHash.size() == HASH_LENGTH) {
    queuePacket(PROTO_EDONKEY, OP_FILEREQANSNOFIL, fileHash);
  }
}

bool SharedResponder::queueFileNameAnswer(const std::string& fileHash)
{
  auto file = findFile(fileHash);
  if (!file) {
    queueNoFile(fileHash);
    return false;
  }
  queuePacket(PROTO_EDONKEY, OP_REQFILENAMEANSWER,
              createSharedFileNameAnswerPayload(*file));
  return true;
}

bool SharedResponder::queueFileStatusAnswer(const std::string& fileHash)
{
  auto file = findFile(fileHash);
  if (!file) {
    queueNoFile(fileHash);
    return false;
  }
  queuePacket(PROTO_EDONKEY, OP_FILESTATUS,
              createSharedFileStatusPayload(*file));
  return true;
}

bool SharedResponder::queueHashSetAnswer(const std::string& fileHash)
{
  auto file = findFile(fileHash);
  if (!file) {
    queueNoFile(fileHash);
    return false;
  }
  std::string payload;
  if (!createSharedFileHashSetPayload(payload, *file)) {
    return false;
  }
  queuePacket(PROTO_EDONKEY, OP_HASHSETANSWER, payload);
  return true;
}

bool SharedResponder::queueSourceExchangeAnswer(const std::string& fileHash,
                                                uint8_t version)
{
  if (!hasFile(fileHash)) {
    return false;
  }
  std::vector<SourceExchangeEntry> entries;
  if (version >= 2) {
    queuePacket(PROTO_EMULE, OP_ANSWERSOURCES2,
                createAnswerSources2Payload(fileHash, version, entries));
  }
  else {
    queuePacket(PROTO_EMULE, OP_ANSWERSOURCES,
                createAnswerSourcesPayload(fileHash, version, entries));
  }
  return true;
}

bool SharedResponder::queueAichFileHashAnswer(const std::string& fileHash)
{
  auto file = findFile(fileHash);
  if (!file || file->aichRootHash.empty()) {
    return false;
  }
  queuePacket(PROTO_EMULE, OP_AICHFILEHASHANS,
              createAichFileHashAnswerPayload(fileHash, file->aichRootHash));
  return true;
}

bool SharedResponder::queueAichAnswer(const std::string& fileHash,
                                      const std::string& requestPayload)
{
  AichRequest request;
  if (!parseAichRequestPayload(request, requestPayload, fileHash)) {
    return false;
  }
  auto file = findFile(fileHash);
  if (!file || file->aichRootHash.empty() ||
      request.rootHash != file->aichRootHash) {
    queuePacket(PROTO_EMULE, OP_AICHANSWER, fileHash);
    return false;
  }
  std::string payload;
  if (!createSharedFileAichAnswerPayload(payload, *file, request.partIndex,
                                         request.rootHash)) {
    queuePacket(PROTO_EMULE, OP_AICHANSWER, fileHash);
    return false;
  }
  queuePacket(PROTO_EMULE, OP_AICHANSWER, payload);
  return true;
}

bool SharedResponder::queuePartAnswers(const std::string& requestPayload,
                                       bool use64BitOffsets)
{
  std::vector<PartRange> ranges;
  std::string fileHash;
  if (!parsePartRequestPayload(ranges, fileHash, requestPayload,
                               use64BitOffsets)) {
    return false;
  }
  auto file = findFile(fileHash);
  if (!file) {
    queueNoFile(fileHash);
    return false;
  }
  for (const auto& range : ranges) {
    std::string payload;
    if (!createSharedFilePartPayload(payload, *file, range,
                                     use64BitOffsets)) {
      continue;
    }
    queuePacket(PROTO_EDONKEY,
                use64BitOffsets ? OP_SENDINGPART_I64 : OP_SENDINGPART,
                payload);
  }
  return true;
}

} // namespace ed2k

} // namespace aria2
