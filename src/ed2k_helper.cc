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
#include "ed2k_helper.h"

#include <algorithm>
#include <limits>
#include <sstream>
#include <zlib.h>

#include "DlAbortEx.h"
#include "Ed2kKadState.h"
#include "fmt.h"
#include "util.h"

namespace aria2 {

namespace ed2k {

namespace {

void validateHashLength(const std::string& hash);

std::string ipv4FromEndpoint(uint32_t ip)
{
  return fmt("%u.%u.%u.%u", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff,
             (ip >> 24) & 0xff);
}

uint32_t ipv4ToEndpointValue(const std::string& host)
{
  uint32_t value = 0;
  std::stringstream ss(host);
  std::string part;
  for (size_t i = 0; i < 4; ++i) {
    if (!std::getline(ss, part, '.')) {
      throw DL_ABORT_EX(fmt("Bad ED2K IPv4 address: %s", host.c_str()));
    }
    uint32_t octet = 0;
    if (!util::parseUIntNoThrow(octet, part) || octet > 255) {
      throw DL_ABORT_EX(fmt("Bad ED2K IPv4 address: %s", host.c_str()));
    }
    value |= octet << (i * 8);
  }
  if (std::getline(ss, part, '.')) {
    throw DL_ABORT_EX(fmt("Bad ED2K IPv4 address: %s", host.c_str()));
  }
  return value;
}

Endpoint readEndpoint(const std::string& data, size_t& offset)
{
  auto ip = readUInt32(readBytes(data, offset, 4).data());
  auto port = readUInt16(readBytes(data, offset, 2).data());
  Endpoint endpoint;
  endpoint.host = ipv4FromEndpoint(ip);
  endpoint.port = port;
  return endpoint;
}

KadContact readKadContact(const std::string& data, size_t& offset)
{
  KadContact contact;
  contact.id = readBytes(data, offset, HASH_LENGTH);
  contact.host = ipv4FromEndpoint(readUInt32(readBytes(data, offset, 4).data()));
  contact.udpPort = readUInt16(readBytes(data, offset, 2).data());
  contact.tcpPort = readUInt16(readBytes(data, offset, 2).data());
  contact.version = readByte(data, offset);
  return contact;
}

KadSearchEntry readKadSearchEntry(const std::string& data, size_t& offset)
{
  KadSearchEntry entry;
  entry.id = readBytes(data, offset, HASH_LENGTH);
  const auto tagCount = readByte(data, offset);
  std::string tagPayload = packUInt32(tagCount);
  for (uint8_t i = 0; i < tagCount; ++i) {
    const auto before = offset;
    readTag(data, offset);
    tagPayload.append(data.begin() + before, data.begin() + offset);
  }
  if (!parseTagList(entry.tags, tagPayload)) {
    throw DL_ABORT_EX("Bad Kad search tags.");
  }
  return entry;
}

std::string packEndpoint(const Endpoint& endpoint)
{
  return packUInt32(ipv4ToEndpointValue(endpoint.host)) +
         packUInt16(endpoint.port);
}

std::string packKadContact(const KadContact& contact)
{
  validateHashLength(contact.id);
  std::string payload = contact.id;
  payload += packUInt32(ipv4ToEndpointValue(contact.host));
  payload += packUInt16(contact.udpPort);
  payload += packUInt16(contact.tcpPort);
  payload.push_back(static_cast<char>(contact.version));
  return payload;
}

std::string packKadSearchEntry(const KadSearchEntry& entry)
{
  validateHashLength(entry.id);
  if (entry.tags.size() > std::numeric_limits<uint8_t>::max()) {
    throw DL_ABORT_EX("Too many Kad search tags.");
  }
  std::string payload = entry.id;
  payload.push_back(static_cast<char>(entry.tags.size()));
  for (const auto& tag : entry.tags) {
    if (tag.valueType == TagValueType::STRING) {
      payload += createStringTag(tag.id, tag.stringValue);
    }
    else if (tag.valueType == TagValueType::UINT) {
      payload += createUInt32Tag(tag.id, static_cast<uint32_t>(tag.intValue));
    }
    else {
      throw DL_ABORT_EX("Unsupported Kad search tag.");
    }
  }
  return payload;
}

void appendSearchAnd(std::string& payload)
{
  payload.push_back('\0');
  payload.push_back('\0');
}

void appendSearchString(std::string& payload, const std::string& value)
{
  if (value.empty()) {
    throw DL_ABORT_EX("ED2K search keyword is empty.");
  }
  if (value.size() > 450) {
    throw DL_ABORT_EX("ED2K search keyword is too long.");
  }
  payload.push_back('\x01');
  payload += packUInt16(static_cast<uint16_t>(value.size()));
  payload += value;
}

void appendSearchTaggedString(std::string& payload, uint8_t tag,
                              const std::string& value)
{
  if (value.size() > 20) {
    throw DL_ABORT_EX("ED2K search tag value is too long.");
  }
  payload.push_back('\x02');
  payload += packUInt16(static_cast<uint16_t>(value.size()));
  payload += value;
  payload += packUInt16(1);
  payload.push_back(static_cast<char>(tag));
}

void appendSearchTaggedNumber(std::string& payload, uint8_t tag,
                              uint8_t searchOperator, uint64_t value,
                              bool supports64Bit)
{
  if (value > std::numeric_limits<uint32_t>::max() && supports64Bit) {
    payload.push_back('\x08');
    payload += packUInt64(value);
  }
  else {
    payload.push_back('\x03');
    payload += packUInt32(static_cast<uint32_t>(
        std::min<uint64_t>(value, std::numeric_limits<uint32_t>::max())));
  }
  payload.push_back(static_cast<char>(searchOperator));
  payload += packUInt16(1);
  payload.push_back(static_cast<char>(tag));
}

uint32_t emuleMiscOptionsValue(const EmuleMiscOptions& options)
{
  return ((options.aichVersion & 0x07u) << 29) |
         (static_cast<uint32_t>(options.unicode) << 28) |
         ((options.udpVersion & 0x0fu) << 24) |
         ((options.dataCompressionVersion & 0x0fu) << 20) |
         ((options.secureIdentVersion & 0x0fu) << 16) |
         ((options.sourceExchange1Version & 0x0fu) << 12) |
         ((options.extendedRequestsVersion & 0x0fu) << 8) |
         (static_cast<uint32_t>(options.multiPacket) << 1);
}

EmuleMiscOptions parseEmuleMiscOptions(uint32_t value)
{
  EmuleMiscOptions options;
  options.aichVersion = (value >> 29) & 0x07;
  options.unicode = (value >> 28) & 0x01;
  options.udpVersion = (value >> 24) & 0x0f;
  options.dataCompressionVersion = (value >> 20) & 0x0f;
  options.secureIdentVersion = (value >> 16) & 0x0f;
  options.sourceExchange1Version = (value >> 12) & 0x0f;
  options.extendedRequestsVersion = (value >> 8) & 0x0f;
  options.multiPacket = (value >> 1) & 0x01;
  return options;
}

uint32_t emuleMiscOptions2Value(const EmuleMiscOptions2& options)
{
  return (static_cast<uint32_t>(options.supportsLargeFiles) << 4) |
         (static_cast<uint32_t>(options.supportsExtendedMultipacket) << 5) |
         (static_cast<uint32_t>(options.supportsSourceExchange2) << 10);
}

EmuleMiscOptions2 parseEmuleMiscOptions2(uint32_t value)
{
  EmuleMiscOptions2 options;
  options.supportsLargeFiles = (value >> 4) & 0x01;
  options.supportsExtendedMultipacket = (value >> 5) & 0x01;
  options.supportsSourceExchange2 = (value >> 10) & 0x01;
  return options;
}

void validateHashLength(const std::string& hash)
{
  if (hash.size() != HASH_LENGTH) {
    throw DL_ABORT_EX("Bad ED2K hash length.");
  }
}

void validateAichHashLength(const std::string& hash)
{
  if (hash.size() != AICH_HASH_LENGTH) {
    throw DL_ABORT_EX("Bad ED2K AICH hash length.");
  }
}

} // namespace

std::vector<Endpoint> parseServerMet(const std::string& data)
{
  size_t offset = 0;
  auto header = readByte(data, offset);
  if (header != 0x0e && header != 0x0f) {
    offset = 0;
  }
  const uint32_t count = readUInt32(readBytes(data, offset, 4).data());
  std::vector<Endpoint> servers;
  servers.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    auto endpoint = readEndpoint(data, offset);
    const uint32_t tagCount = readUInt32(readBytes(data, offset, 4).data());
    for (uint32_t j = 0; j < tagCount; ++j) {
      skipTag(data, offset);
    }
    if (!endpoint.host.empty() && endpoint.port) {
      servers.push_back(std::move(endpoint));
    }
  }
  return servers;
}

std::string createLoginRequestPayload(const std::string& clientHash,
                                      uint16_t listenPort,
                                      const std::string& clientName)
{
  validateHashLength(clientHash);
  std::string payload;
  payload += clientHash;
  payload += packUInt32(0);
  payload += packUInt16(listenPort);
  payload += packUInt32(4);
  payload += createUInt32Tag(0x11, 0x3c);
  payload += createUInt32Tag(0x20, 0x011d);
  payload += createStringTag(0x01, clientName);
  payload += createUInt32Tag(0xfb, (1u << 24) | (1u << 17) | (1u << 7));
  return payload;
}

std::string createGetSourcesPayload(const std::string& fileHash,
                                    int64_t fileSize)
{
  validateHashLength(fileHash);
  if (fileSize < 0) {
    throw DL_ABORT_EX("Bad ED2K file size.");
  }
  std::string payload = fileHash;
  if (fileSize > std::numeric_limits<uint32_t>::max()) {
    payload += packUInt32(0);
    payload += packUInt32(static_cast<uint32_t>(fileSize));
    payload += packUInt32(static_cast<uint32_t>(
        static_cast<uint64_t>(fileSize) >> 32));
  }
  else {
    payload += packUInt32(static_cast<uint32_t>(fileSize));
  }
  return payload;
}

std::string createFoundSourcesPayload(const std::string& fileHash,
                                      const std::vector<Endpoint>& sources)
{
  validateHashLength(fileHash);
  if (sources.size() > 255) {
    throw DL_ABORT_EX("Too many ED2K sources.");
  }
  std::string payload = fileHash;
  payload.push_back(static_cast<char>(sources.size()));
  for (const auto& source : sources) {
    payload += packEndpoint(source);
  }
  return payload;
}

std::vector<Endpoint> parseFoundSourcesPayload(const std::string& payload)
{
  size_t offset = 0;
  readBytes(payload, offset, HASH_LENGTH);
  auto count = readByte(payload, offset);
  std::vector<Endpoint> sources;
  sources.reserve(count);
  for (uint8_t i = 0; i < count; ++i) {
    sources.push_back(readEndpoint(payload, offset));
  }
  return sources;
}

bool parseFoundSourcesPayload(std::vector<Endpoint>& sources,
                              const std::string& payload,
                              const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  size_t offset = 0;
  auto hash = readBytes(payload, offset, HASH_LENGTH);
  if (hash != expectedFileHash) {
    return false;
  }
  auto count = readByte(payload, offset);
  sources.clear();
  sources.reserve(count);
  for (uint8_t i = 0; i < count; ++i) {
    sources.push_back(readEndpoint(payload, offset));
  }
  return true;
}

bool parseFoundSourcesPayload(std::vector<FoundSource>& sources,
                              const std::string& payload,
                              const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  size_t offset = 0;
  auto hash = readBytes(payload, offset, HASH_LENGTH);
  if (hash != expectedFileHash) {
    return false;
  }
  auto count = readByte(payload, offset);
  sources.clear();
  sources.reserve(count);
  for (uint8_t i = 0; i < count; ++i) {
    FoundSource source;
    source.endpoint = readEndpoint(payload, offset);
    source.clientId = ipv4ToEndpointValue(source.endpoint.host);
    source.lowId = source.clientId <= 0x00ffffffu;
    sources.push_back(source);
  }
  return true;
}

std::string createCallbackRequestPayload(uint32_t clientId)
{
  return packUInt32(clientId);
}

bool parseCallbackRequestIncomingPayload(Endpoint& endpoint,
                                         const std::string& payload)
{
  if (payload.size() != 6) {
    return false;
  }
  size_t offset = 0;
  endpoint = readEndpoint(payload, offset);
  return true;
}

bool parseServerIdChangePayload(ServerIdChange& idChange,
                                const std::string& payload)
{
  if (payload.size() < 4) {
    return false;
  }
  if (payload.size() != 4 && payload.size() != 8 && payload.size() != 12 &&
      payload.size() != 16 && payload.size() != 20) {
    return false;
  }
  ServerIdChange parsed;
  size_t offset = 0;
  parsed.clientId = readUInt32(readBytes(payload, offset, 4).data());
  if (payload.size() >= 8) {
    parsed.tcpFlags = readUInt32(readBytes(payload, offset, 4).data());
  }
  if (payload.size() >= 12) {
    parsed.auxPort = readUInt32(readBytes(payload, offset, 4).data());
  }
  if (payload.size() >= 16) {
    const auto reportedIp = readUInt32(readBytes(payload, offset, 4).data());
    if (reportedIp > 0x00ffffffu) {
      parsed.ipAddress = ipv4FromEndpoint(reportedIp);
    }
  }
  if (payload.size() >= 20) {
    readBytes(payload, offset, 4);
  }
  parsed.highId = parsed.clientId > 0x00ffffffu;
  if (parsed.ipAddress.empty() && parsed.highId) {
    parsed.ipAddress = ipv4FromEndpoint(parsed.clientId);
  }
  idChange = std::move(parsed);
  return true;
}

bool parseServerStatusPayload(ServerStatus& status,
                              const std::string& payload)
{
  if (payload.size() != 8 && payload.size() != 12 && payload.size() != 16 &&
      payload.size() != 24 && payload.size() != 28 && payload.size() != 32 &&
      payload.size() != 40) {
    return false;
  }
  size_t offset = 0;
  ServerStatus parsed;
  if (payload.size() != 8) {
    parsed.challenge = readUInt32(readBytes(payload, offset, 4).data());
  }
  parsed.users = readUInt32(readBytes(payload, offset, 4).data());
  parsed.files = readUInt32(readBytes(payload, offset, 4).data());
  if (payload.size() >= 16) {
    parsed.maxUsers = readUInt32(readBytes(payload, offset, 4).data());
  }
  if (payload.size() >= 24) {
    parsed.softFiles = readUInt32(readBytes(payload, offset, 4).data());
    parsed.hardFiles = readUInt32(readBytes(payload, offset, 4).data());
  }
  if (payload.size() >= 28) {
    parsed.udpFlags = readUInt32(readBytes(payload, offset, 4).data());
  }
  if (payload.size() >= 32) {
    parsed.lowIdUsers = readUInt32(readBytes(payload, offset, 4).data());
  }
  if (payload.size() >= 40) {
    parsed.udpObfuscationPort =
        readUInt16(readBytes(payload, offset, 2).data());
    parsed.tcpObfuscationPort =
        readUInt16(readBytes(payload, offset, 2).data());
    parsed.udpKey = readUInt32(readBytes(payload, offset, 4).data());
  }
  status = parsed;
  return true;
}

bool parseServerMessagePayload(std::string& message,
                               const std::string& payload)
{
  if (payload.size() < 2) {
    return false;
  }
  const auto length = readUInt16(payload.data());
  if (length != payload.size() - 2) {
    return false;
  }
  message.assign(payload.begin() + 2, payload.end());
  return true;
}

bool parseServerListPayload(std::vector<Endpoint>& servers,
                            const std::string& payload)
{
  if (payload.empty()) {
    return false;
  }
  size_t offset = 0;
  const auto count = readByte(payload, offset);
  if (payload.size() - offset != static_cast<size_t>(count) * 6) {
    return false;
  }
  servers.clear();
  servers.reserve(count);
  for (uint8_t i = 0; i < count; ++i) {
    servers.push_back(readEndpoint(payload, offset));
  }
  return true;
}

namespace {

constexpr char SERVER_STATE_MAGIC[] = "A2ED2KSRV";
constexpr uint32_t SERVER_STATE_VERSION = 1;

void appendByte(std::string& payload, uint8_t value);
void appendInt64(std::string& payload, int64_t value);
int64_t readInt64(const std::string& payload, size_t& offset);
void appendString(std::string& payload, const std::string& value);
std::string readString(const std::string& payload, size_t& offset);

void appendServerStateEndpoint(std::string& payload, const Endpoint& endpoint)
{
  appendString(payload, endpoint.host);
  payload += packUInt16(endpoint.port);
}

Endpoint readServerStateEndpoint(const std::string& payload, size_t& offset)
{
  Endpoint endpoint;
  endpoint.host = readString(payload, offset);
  endpoint.port = readUInt16(readBytes(payload, offset, 2).data());
  return endpoint;
}

} // namespace

std::string createServerStatePayload(const ServerState& state)
{
  std::string payload;
  payload.append(SERVER_STATE_MAGIC, sizeof(SERVER_STATE_MAGIC) - 1);
  payload += packUInt32(SERVER_STATE_VERSION);
  appendServerStateEndpoint(payload, state.endpoint);
  appendByte(payload, state.connected ? 1 : 0);
  appendByte(payload, state.handshakeCompleted ? 1 : 0);
  payload += packUInt32(state.clientId);
  appendByte(payload, state.highId ? 1 : 0);
  appendString(payload, state.ipAddress);
  payload += packUInt32(state.tcpFlags);
  payload += packUInt32(state.users);
  payload += packUInt32(state.files);
  payload += packUInt32(state.maxUsers);
  payload += packUInt32(state.softFiles);
  payload += packUInt32(state.hardFiles);
  payload += packUInt32(state.udpFlags);
  payload += packUInt32(state.lowIdUsers);
  payload += packUInt16(state.udpObfuscationPort);
  payload += packUInt16(state.tcpObfuscationPort);
  payload += packUInt32(state.udpKey);
  payload += packUInt32(state.udpStatusChallenge);
  appendInt64(payload, state.lastUdpStatusTime);
  payload += packUInt32(state.failCount);
  appendInt64(payload, state.lastFailureTime);
  appendInt64(payload, state.nextRetryTime);
  appendString(payload, state.lastMessage);
  return payload;
}

bool parseServerStatePayload(ServerState& state, const std::string& payload)
{
  try {
    size_t offset = 0;
    const auto magic =
        readBytes(payload, offset, sizeof(SERVER_STATE_MAGIC) - 1);
    if (magic != std::string(SERVER_STATE_MAGIC,
                             sizeof(SERVER_STATE_MAGIC) - 1)) {
      return false;
    }
    const auto version = readUInt32(readBytes(payload, offset, 4).data());
    if (version != SERVER_STATE_VERSION) {
      return false;
    }
    ServerState parsed;
    parsed.endpoint = readServerStateEndpoint(payload, offset);
    parsed.connected = readByte(payload, offset) != 0;
    parsed.handshakeCompleted = readByte(payload, offset) != 0;
    parsed.clientId = readUInt32(readBytes(payload, offset, 4).data());
    parsed.highId = readByte(payload, offset) != 0;
    parsed.ipAddress = readString(payload, offset);
    parsed.tcpFlags = readUInt32(readBytes(payload, offset, 4).data());
    parsed.users = readUInt32(readBytes(payload, offset, 4).data());
    parsed.files = readUInt32(readBytes(payload, offset, 4).data());
    parsed.maxUsers = readUInt32(readBytes(payload, offset, 4).data());
    parsed.softFiles = readUInt32(readBytes(payload, offset, 4).data());
    parsed.hardFiles = readUInt32(readBytes(payload, offset, 4).data());
    parsed.udpFlags = readUInt32(readBytes(payload, offset, 4).data());
    parsed.lowIdUsers = readUInt32(readBytes(payload, offset, 4).data());
    parsed.udpObfuscationPort =
        readUInt16(readBytes(payload, offset, 2).data());
    parsed.tcpObfuscationPort =
        readUInt16(readBytes(payload, offset, 2).data());
    parsed.udpKey = readUInt32(readBytes(payload, offset, 4).data());
    parsed.udpStatusChallenge =
        readUInt32(readBytes(payload, offset, 4).data());
    parsed.lastUdpStatusTime = readInt64(payload, offset);
    parsed.failCount = readUInt32(readBytes(payload, offset, 4).data());
    parsed.lastFailureTime = readInt64(payload, offset);
    parsed.nextRetryTime = readInt64(payload, offset);
    parsed.lastMessage = readString(payload, offset);
    if (offset != payload.size()) {
      return false;
    }
    state = std::move(parsed);
    return true;
  }
  catch (DlAbortEx&) {
    return false;
  }
}

bool parseSearchResultPayload(SearchResult& result, const std::string& payload,
                              const std::string& sourceNetwork)
{
  if (payload.size() < 4) {
    return false;
  }
  size_t offset = 0;
  const auto count = readUInt32(readBytes(payload, offset, 4).data());
  if (count > 10000) {
    return false;
  }
  result.entries.clear();
  result.entries.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    if (payload.size() - offset < HASH_LENGTH + 6) {
      return false;
    }
    SearchResultEntry entry;
    entry.hash = readBytes(payload, offset, HASH_LENGTH);
    readBytes(payload, offset, 4);
    readBytes(payload, offset, 2);
    std::vector<Tag> tags;
    const auto tagCount = readUInt32(readBytes(payload, offset, 4).data());
    std::string tagPayload;
    tagPayload += packUInt32(tagCount);
    for (uint32_t tagIndex = 0; tagIndex < tagCount; ++tagIndex) {
      const auto before = offset;
      readTag(payload, offset);
      tagPayload.append(payload.begin() + before, payload.begin() + offset);
    }
    if (!parseTagList(tags, tagPayload)) {
      return false;
    }
    entry.tags = tags;
    uint64_t sizeLow = 0;
    uint64_t sizeHigh = 0;
    for (const auto& tag : tags) {
      if (tag.valueType == TagValueType::STRING) {
        if (tag.id == 0x01) {
          entry.name = tag.stringValue;
        }
        else if (tag.id == 0x03) {
          entry.fileType = tag.stringValue;
        }
        else if (tag.id == 0x04) {
          entry.extension = tag.stringValue;
        }
        else if (tag.id == 0xd0 || tag.name == "Artist") {
          entry.mediaArtist = tag.stringValue;
        }
        else if (tag.id == 0xd1 || tag.name == "Album") {
          entry.mediaAlbum = tag.stringValue;
        }
        else if (tag.id == 0xd2 || tag.name == "Title") {
          entry.mediaTitle = tag.stringValue;
        }
        else if (tag.id == 0xd3 || tag.name == "length") {
          entry.mediaLength = tag.stringValue;
        }
        else if (tag.id == 0xd5) {
          entry.mediaCodec = tag.stringValue;
        }
      }
      else if (tag.valueType == TagValueType::UINT) {
        if (tag.id == 0x02) {
          sizeLow = tag.intValue;
        }
        else if (tag.id == 0x3a) {
          sizeHigh = tag.intValue;
        }
        else if (tag.id == 0x15) {
          entry.sourceCount = static_cast<uint32_t>(tag.intValue);
        }
        else if (tag.id == 0x30) {
          entry.completeSourceCount = static_cast<uint32_t>(tag.intValue);
        }
        else if (tag.id == 0xd3) {
          entry.mediaLength = util::uitos(tag.intValue);
        }
        else if (tag.id == 0xd4 || tag.name == "bitrate") {
          entry.mediaBitrate = static_cast<uint32_t>(tag.intValue);
        }
      }
    }
    const uint64_t size = sizeLow | (sizeHigh << 32);
    if (size > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
      return false;
    }
    entry.size = static_cast<int64_t>(size);
    entry.sourceNetwork = sourceNetwork;
    Link link;
    link.type = LinkType::FILE;
    link.name = entry.name;
    link.size = entry.size;
    link.hash = entry.hash;
    entry.ed2kLink = toFileLink(link);
    result.entries.push_back(std::move(entry));
  }
  result.moreResults = false;
  if (payload.size() - offset == 1) {
    result.moreResults = readByte(payload, offset) != 0;
  }
  return offset == payload.size();
}

std::string pickKadKeyword(const std::string& query)
{
  auto value = util::toLower(query);
  std::string best;
  std::string current;
  for (const auto c : value) {
    if (util::isAlpha(c) || util::isDigit(c)) {
      current.push_back(c);
      continue;
    }
    if (current.size() >= 3 && current.size() > best.size()) {
      best = current;
    }
    current.clear();
  }
  if (current.size() >= 3 && current.size() > best.size()) {
    best = current;
  }
  return best;
}

std::string createKadKeywordTarget(const std::string& query)
{
  auto keyword = pickKadKeyword(query);
  if (keyword.empty()) {
    throw DL_ABORT_EX("ED2K Kad search keyword is empty.");
  }
  return md4Digest(keyword);
}

std::string createSearchRequestPayload(const SearchQuery& query,
                                       bool supports64Bit)
{
  struct SearchTerm {
    enum class Type { KEYWORD, STRING_TAG, NUMBER_TAG };
    Type type = Type::KEYWORD;
    std::string text;
    uint8_t tag = 0;
    uint8_t searchOperator = 0;
    uint64_t number = 0;
  };

  std::vector<SearchTerm> terms;
  SearchTerm keyword;
  keyword.type = SearchTerm::Type::KEYWORD;
  keyword.text = query.keyword;
  terms.push_back(keyword);
  if (!query.fileType.empty()) {
    SearchTerm term;
    term.type = SearchTerm::Type::STRING_TAG;
    term.tag = 0x03;
    term.text = query.fileType;
    terms.push_back(term);
  }
  if (query.minSize > 0) {
    SearchTerm term;
    term.type = SearchTerm::Type::NUMBER_TAG;
    term.tag = 0x02;
    term.searchOperator = 0x01;
    term.number = static_cast<uint64_t>(query.minSize);
    terms.push_back(term);
  }
  if (query.maxSize > 0) {
    SearchTerm term;
    term.type = SearchTerm::Type::NUMBER_TAG;
    term.tag = 0x02;
    term.searchOperator = 0x02;
    term.number = static_cast<uint64_t>(query.maxSize);
    terms.push_back(term);
  }
  if (query.minSourceCount > 0) {
    SearchTerm term;
    term.type = SearchTerm::Type::NUMBER_TAG;
    term.tag = 0x15;
    term.searchOperator = 0x01;
    term.number = query.minSourceCount;
    terms.push_back(term);
  }
  if (query.minCompleteSourceCount > 0) {
    SearchTerm term;
    term.type = SearchTerm::Type::NUMBER_TAG;
    term.tag = 0x30;
    term.searchOperator = 0x01;
    term.number = query.minCompleteSourceCount;
    terms.push_back(term);
  }
  if (!query.extension.empty()) {
    SearchTerm term;
    term.type = SearchTerm::Type::STRING_TAG;
    term.tag = 0x04;
    term.text = query.extension;
    terms.push_back(term);
  }
  if (terms.size() > 30) {
    throw DL_ABORT_EX("Too many ED2K search terms.");
  }

  std::string payload;
  for (size_t i = 0; i < terms.size(); ++i) {
    if (i + 1 < terms.size()) {
      appendSearchAnd(payload);
    }
    const auto& term = terms[i];
    switch (term.type) {
    case SearchTerm::Type::KEYWORD:
      appendSearchString(payload, term.text);
      break;
    case SearchTerm::Type::STRING_TAG:
      appendSearchTaggedString(payload, term.tag, term.text);
      break;
    case SearchTerm::Type::NUMBER_TAG:
      appendSearchTaggedNumber(payload, term.tag, term.searchOperator,
                               term.number, supports64Bit);
      break;
    }
  }
  return payload;
}

std::string createKadHelloPayload(const std::string& id, uint16_t tcpPort,
                                  uint8_t version)
{
  validateHashLength(id);
  std::string payload = id;
  payload += packUInt16(tcpPort);
  payload.push_back(static_cast<char>(version));
  payload.push_back('\0');
  return payload;
}

bool parseKadHelloPayload(KadHello& hello, const std::string& payload)
{
  if (payload.size() < HASH_LENGTH + 4) {
    return false;
  }
  size_t offset = 0;
  hello.id = readBytes(payload, offset, HASH_LENGTH);
  hello.tcpPort = readUInt16(readBytes(payload, offset, 2).data());
  hello.version = readByte(payload, offset);
  const auto tagCount = readByte(payload, offset);
  if (payload.size() - offset < tagCount * 2) {
    return false;
  }

  std::string tagPayload = packUInt32(tagCount);
  tagPayload.append(payload.begin() + offset, payload.end());
  if (!parseTagList(hello.tags, tagPayload)) {
    return false;
  }
  offset = payload.size();
  return true;
}

std::string createKadBootstrapResponsePayload(
    const std::string& id, uint16_t tcpPort, uint8_t version,
    const std::vector<KadContact>& contacts)
{
  validateHashLength(id);
  if (contacts.size() > std::numeric_limits<uint16_t>::max()) {
    throw DL_ABORT_EX("Too many ED2K Kad contacts.");
  }
  std::string payload = id;
  payload += packUInt16(tcpPort);
  payload.push_back(static_cast<char>(version));
  payload += packUInt16(static_cast<uint16_t>(contacts.size()));
  for (const auto& contact : contacts) {
    payload += packKadContact(contact);
  }
  return payload;
}

bool parseKadBootstrapResponsePayload(KadBootstrapResponse& response,
                                      const std::string& payload)
{
  if (payload.size() < HASH_LENGTH + 5) {
    return false;
  }
  size_t offset = 0;
  response.id = readBytes(payload, offset, HASH_LENGTH);
  response.tcpPort = readUInt16(readBytes(payload, offset, 2).data());
  response.version = readByte(payload, offset);
  const auto count = readUInt16(readBytes(payload, offset, 2).data());
  if (payload.size() - offset != static_cast<size_t>(count) * 25) {
    return false;
  }
  response.contacts.clear();
  response.contacts.reserve(count);
  for (uint16_t i = 0; i < count; ++i) {
    response.contacts.push_back(readKadContact(payload, offset));
  }
  return true;
}

std::string createKadRequestPayload(uint8_t searchType,
                                    const std::string& targetId,
                                    const std::string& receiverId)
{
  validateHashLength(targetId);
  validateHashLength(receiverId);
  std::string payload;
  payload.push_back(static_cast<char>(searchType));
  payload += targetId;
  payload += receiverId;
  return payload;
}

bool parseKadRequestPayload(KadRequest& request, const std::string& payload)
{
  if (payload.size() != HASH_LENGTH * 2 + 1) {
    return false;
  }
  size_t offset = 0;
  request.searchType = readByte(payload, offset);
  request.targetId = readBytes(payload, offset, HASH_LENGTH);
  request.receiverId = readBytes(payload, offset, HASH_LENGTH);
  return true;
}

std::string createKadResponsePayload(const std::string& targetId,
                                     const std::vector<KadContact>& contacts)
{
  validateHashLength(targetId);
  if (contacts.size() > std::numeric_limits<uint8_t>::max()) {
    throw DL_ABORT_EX("Too many ED2K Kad response contacts.");
  }
  std::string payload = targetId;
  payload.push_back(static_cast<char>(contacts.size()));
  for (const auto& contact : contacts) {
    payload += packKadContact(contact);
  }
  return payload;
}

bool parseKadResponsePayload(KadResponse& response,
                             const std::string& payload)
{
  if (payload.size() < HASH_LENGTH + 1) {
    return false;
  }
  size_t offset = 0;
  response.targetId = readBytes(payload, offset, HASH_LENGTH);
  const auto count = readByte(payload, offset);
  if (payload.size() - offset != static_cast<size_t>(count) * 25) {
    return false;
  }
  response.contacts.clear();
  response.contacts.reserve(count);
  for (uint8_t i = 0; i < count; ++i) {
    response.contacts.push_back(readKadContact(payload, offset));
  }
  return true;
}

std::string createKadSearchSourcesRequestPayload(const std::string& targetId,
                                                 uint16_t startPosition,
                                                 uint64_t size)
{
  validateHashLength(targetId);
  return targetId + packUInt16(startPosition) + packUInt64(size);
}

bool parseKadSearchSourcesRequestPayload(KadSearchSourcesRequest& request,
                                         const std::string& payload)
{
  if (payload.size() != HASH_LENGTH + 10) {
    return false;
  }
  size_t offset = 0;
  request.targetId = readBytes(payload, offset, HASH_LENGTH);
  request.startPosition = readUInt16(readBytes(payload, offset, 2).data());
  request.size = readUInt64(readBytes(payload, offset, 8).data());
  return true;
}

std::string createKadSearchKeysRequestPayload(const std::string& targetId,
                                              uint16_t startPosition)
{
  validateHashLength(targetId);
  return targetId + packUInt16(startPosition);
}

bool parseKadSearchResultPayload(KadSearchResult& result,
                                 const std::string& payload)
{
  if (payload.size() < HASH_LENGTH * 2 + 2) {
    return false;
  }
  try {
    size_t offset = 0;
    result.sourceId = readBytes(payload, offset, HASH_LENGTH);
    result.targetId = readBytes(payload, offset, HASH_LENGTH);
    const auto count = readUInt16(readBytes(payload, offset, 2).data());
    result.entries.clear();
    result.entries.reserve(count);
    for (uint16_t i = 0; i < count; ++i) {
      result.entries.push_back(readKadSearchEntry(payload, offset));
    }
    return offset == payload.size();
  }
  catch (DlAbortEx&) {
    return false;
  }
}

std::vector<Endpoint> extractKadSourceEndpoints(const KadSearchResult& result)
{
  std::vector<Endpoint> endpoints;
  for (const auto& entry : result.entries) {
    uint32_t ip = 0;
    uint16_t port = 0;
    uint64_t sourceType = 0;
    bool hasIp = false;
    bool hasPort = false;
    for (const auto& tag : entry.tags) {
      if (tag.valueType != TagValueType::UINT) {
        continue;
      }
      if (tag.id == 0xfe) {
        ip = static_cast<uint32_t>(tag.intValue);
        hasIp = true;
      }
      else if (tag.id == 0xfd) {
        port = static_cast<uint16_t>(tag.intValue);
        hasPort = true;
      }
      else if (tag.id == 0xff) {
        sourceType = tag.intValue;
      }
    }
    if (hasIp && hasPort && port != 0 &&
        (sourceType == 0 || sourceType == 1 || sourceType == 4)) {
      Endpoint endpoint;
      endpoint.host = ipv4FromEndpoint(ip);
      endpoint.port = port;
      endpoints.push_back(std::move(endpoint));
    }
  }
  return endpoints;
}

std::vector<SearchResultEntry> kadSearchEntriesToSearchResults(
    const std::vector<KadSearchEntry>& entries,
    const std::string& sourceNetwork)
{
  std::vector<SearchResultEntry> results;
  results.reserve(entries.size());
  for (const auto& item : entries) {
    if (item.id.size() != HASH_LENGTH) {
      continue;
    }
    SearchResultEntry entry;
    entry.hash = item.id;
    entry.tags = item.tags;
    uint64_t sizeLow = 0;
    uint64_t sizeHigh = 0;
    for (const auto& tag : item.tags) {
      if (tag.valueType == TagValueType::STRING) {
        if (tag.id == 0x01) {
          entry.name = tag.stringValue;
        }
        else if (tag.id == 0x03) {
          entry.fileType = tag.stringValue;
        }
        else if (tag.id == 0x04) {
          entry.extension = tag.stringValue;
        }
        else if (tag.id == 0xd0 || tag.name == "Artist") {
          entry.mediaArtist = tag.stringValue;
        }
        else if (tag.id == 0xd1 || tag.name == "Album") {
          entry.mediaAlbum = tag.stringValue;
        }
        else if (tag.id == 0xd2 || tag.name == "Title") {
          entry.mediaTitle = tag.stringValue;
        }
        else if (tag.id == 0xd3 || tag.name == "length") {
          entry.mediaLength = tag.stringValue;
        }
        else if (tag.id == 0xd5) {
          entry.mediaCodec = tag.stringValue;
        }
      }
      else if (tag.valueType == TagValueType::UINT) {
        if (tag.id == 0x02) {
          sizeLow = tag.intValue;
        }
        else if (tag.id == 0x3a) {
          sizeHigh = tag.intValue;
        }
        else if (tag.id == 0x15) {
          entry.sourceCount = static_cast<uint32_t>(tag.intValue);
        }
        else if (tag.id == 0x30) {
          entry.completeSourceCount = static_cast<uint32_t>(tag.intValue);
        }
        else if (tag.id == 0xd3) {
          entry.mediaLength = util::uitos(tag.intValue);
        }
        else if (tag.id == 0xd4 || tag.name == "bitrate") {
          entry.mediaBitrate = static_cast<uint32_t>(tag.intValue);
        }
      }
    }
    const uint64_t size = sizeLow | (sizeHigh << 32);
    if (size > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
      continue;
    }
    entry.size = static_cast<int64_t>(size);
    entry.sourceNetwork = sourceNetwork;
    if (!entry.name.empty() && entry.size > 0) {
      Link link;
      link.type = LinkType::FILE;
      link.name = entry.name;
      link.size = entry.size;
      link.hash = entry.hash;
      entry.ed2kLink = toFileLink(link);
    }
    results.push_back(std::move(entry));
  }
  return results;
}

std::string createKadPublishSourceRequestPayload(const std::string& fileId,
                                                 const Endpoint& source,
                                                 const std::string& sourceId)
{
  validateHashLength(fileId);
  validateHashLength(sourceId);
  KadSearchEntry entry;
  entry.id = sourceId;

  Tag sourceType;
  sourceType.id = 0xff;
  sourceType.valueType = TagValueType::UINT;
  sourceType.intValue = 1;
  entry.tags.push_back(sourceType);

  Tag sourceIp;
  sourceIp.id = 0xfe;
  sourceIp.valueType = TagValueType::UINT;
  sourceIp.intValue = ipv4ToEndpointValue(source.host);
  entry.tags.push_back(sourceIp);

  Tag sourcePort;
  sourcePort.id = 0xfd;
  sourcePort.valueType = TagValueType::UINT;
  sourcePort.intValue = source.port;
  entry.tags.push_back(sourcePort);

  return fileId + packKadSearchEntry(entry);
}

bool parseKadPublishSourceRequestPayload(KadPublishSourceRequest& request,
                                         const std::string& payload)
{
  if (payload.size() < HASH_LENGTH * 2 + 1) {
    return false;
  }
  try {
    size_t offset = 0;
    request.fileId = readBytes(payload, offset, HASH_LENGTH);
    request.source = readKadSearchEntry(payload, offset);
    return offset == payload.size();
  }
  catch (DlAbortEx&) {
    return false;
  }
}

std::string createKadPublishResultPayload(const std::string& fileId,
                                          uint8_t count)
{
  validateHashLength(fileId);
  std::string payload = fileId;
  payload.push_back(static_cast<char>(count));
  return payload;
}

bool parseKadPublishResultPayload(KadPublishResult& result,
                                  const std::string& payload)
{
  if (payload.size() != HASH_LENGTH + 1) {
    return false;
  }
  size_t offset = 0;
  result.fileId = readBytes(payload, offset, HASH_LENGTH);
  result.count = readByte(payload, offset);
  return true;
}

std::string createKadFirewalledRequestPayload(uint16_t tcpPort,
                                              const std::string& id,
                                              uint8_t options)
{
  validateHashLength(id);
  std::string payload = packUInt16(tcpPort);
  payload += id;
  payload.push_back(static_cast<char>(options));
  return payload;
}

bool parseKadFirewalledRequestPayload(KadFirewalledRequest& request,
                                      const std::string& payload)
{
  if (payload.size() != HASH_LENGTH + 3) {
    return false;
  }
  size_t offset = 0;
  request.tcpPort = readUInt16(readBytes(payload, offset, 2).data());
  request.id = readBytes(payload, offset, HASH_LENGTH);
  request.options = readByte(payload, offset);
  return true;
}

std::string createKadFirewalledResponsePayload(const std::string& ipAddress)
{
  return packUInt32(ipv4ToEndpointValue(ipAddress));
}

bool parseKadFirewalledResponsePayload(KadFirewalledResponse& response,
                                       const std::string& payload)
{
  if (payload.size() != 4) {
    return false;
  }
  response.ipAddress = ipv4FromEndpoint(readUInt32(payload.data()));
  return true;
}

std::string createKadFirewalledUdpPayload(uint8_t errorCode,
                                          uint16_t tcpPort)
{
  std::string payload;
  payload.push_back(static_cast<char>(errorCode));
  payload += packUInt16(tcpPort);
  return payload;
}

bool parseKadFirewalledUdpPayload(KadFirewalledUdp& response,
                                  const std::string& payload)
{
  if (payload.size() != 3) {
    return false;
  }
  size_t offset = 0;
  response.errorCode = readByte(payload, offset);
  response.tcpPort = readUInt16(readBytes(payload, offset, 2).data());
  return true;
}

namespace {

constexpr char KAD_ROUTING_STATE_MAGIC[] = "A2ED2KKAD";
constexpr uint32_t KAD_ROUTING_STATE_VERSION = 1;

void appendByte(std::string& payload, uint8_t value)
{
  payload.push_back(static_cast<char>(value));
}

void appendInt64(std::string& payload, int64_t value)
{
  payload += packUInt64(static_cast<uint64_t>(value));
}

int64_t readInt64(const std::string& payload, size_t& offset)
{
  return static_cast<int64_t>(readUInt64(readBytes(payload, offset, 8).data()));
}

void appendString(std::string& payload, const std::string& value)
{
  if (value.size() > std::numeric_limits<uint16_t>::max()) {
    throw DL_ABORT_EX("ED2K Kad routing state string is too large.");
  }
  payload += packUInt16(static_cast<uint16_t>(value.size()));
  payload += value;
}

std::string readString(const std::string& payload, size_t& offset)
{
  const auto size = readUInt16(readBytes(payload, offset, 2).data());
  return readBytes(payload, offset, size);
}

void appendEndpoint(std::string& payload, const Endpoint& endpoint)
{
  appendString(payload, endpoint.host);
  payload += packUInt16(endpoint.port);
}

Endpoint readStateEndpoint(const std::string& payload, size_t& offset)
{
  Endpoint endpoint;
  endpoint.host = readString(payload, offset);
  endpoint.port = readUInt16(readBytes(payload, offset, 2).data());
  return endpoint;
}

void appendKadRoutingNode(std::string& payload, const KadRoutingNode& node)
{
  validateHashLength(node.contact.id);
  payload += node.contact.id;
  appendString(payload, node.contact.host);
  payload += packUInt16(node.contact.udpPort);
  payload += packUInt16(node.contact.tcpPort);
  appendByte(payload, node.contact.version);
  appendByte(payload, node.confirmed ? 1 : 0);
  appendByte(payload, node.seed ? 1 : 0);
  payload += packUInt32(node.failCount);
  appendInt64(payload, node.firstSeen);
  appendInt64(payload, node.lastSeen);
}

KadRoutingNode readKadRoutingNode(const std::string& payload, size_t& offset)
{
  KadRoutingNode node;
  node.contact.id = readBytes(payload, offset, HASH_LENGTH);
  node.contact.host = readString(payload, offset);
  node.contact.udpPort = readUInt16(readBytes(payload, offset, 2).data());
  node.contact.tcpPort = readUInt16(readBytes(payload, offset, 2).data());
  node.contact.version = readByte(payload, offset);
  node.confirmed = readByte(payload, offset) != 0;
  node.seed = readByte(payload, offset) != 0;
  node.failCount = readUInt32(readBytes(payload, offset, 4).data());
  node.firstSeen = readInt64(payload, offset);
  node.lastSeen = readInt64(payload, offset);
  return node;
}

void appendKadRoutingNodes(std::string& payload,
                           const std::vector<KadRoutingNode>& nodes)
{
  if (nodes.size() > std::numeric_limits<uint32_t>::max()) {
    throw DL_ABORT_EX("ED2K Kad routing state has too many nodes.");
  }
  payload += packUInt32(static_cast<uint32_t>(nodes.size()));
  for (const auto& node : nodes) {
    appendKadRoutingNode(payload, node);
  }
}

std::vector<KadRoutingNode> readKadRoutingNodes(const std::string& payload,
                                                size_t& offset)
{
  const auto count = readUInt32(readBytes(payload, offset, 4).data());
  std::vector<KadRoutingNode> nodes;
  nodes.reserve(count);
  for (uint32_t i = 0; i < count; ++i) {
    nodes.push_back(readKadRoutingNode(payload, offset));
  }
  return nodes;
}

} // namespace

std::string createKadRoutingStatePayload(const KadRoutingSnapshot& snapshot)
{
  validateHashLength(snapshot.selfId);
  if (snapshot.buckets.size() > std::numeric_limits<uint16_t>::max() ||
      snapshot.routerNodes.size() > std::numeric_limits<uint16_t>::max()) {
    throw DL_ABORT_EX("ED2K Kad routing state is too large.");
  }
  std::string payload;
  payload.append(KAD_ROUTING_STATE_MAGIC,
                 sizeof(KAD_ROUTING_STATE_MAGIC) - 1);
  payload += packUInt32(KAD_ROUTING_STATE_VERSION);
  payload += snapshot.selfId;
  appendInt64(payload, snapshot.lastBootstrap);
  appendInt64(payload, snapshot.lastRefresh);
  appendInt64(payload, snapshot.lastSelfRefresh);
  payload += packUInt16(static_cast<uint16_t>(snapshot.routerNodes.size()));
  for (const auto& endpoint : snapshot.routerNodes) {
    appendEndpoint(payload, endpoint);
  }
  payload += packUInt16(static_cast<uint16_t>(snapshot.buckets.size()));
  for (const auto& bucket : snapshot.buckets) {
    appendInt64(payload, bucket.lastActive);
    appendKadRoutingNodes(payload, bucket.live);
    appendKadRoutingNodes(payload, bucket.replacements);
  }
  return payload;
}

bool parseKadRoutingStatePayload(KadRoutingSnapshot& snapshot,
                                 const std::string& payload)
{
  try {
    size_t offset = 0;
    const auto magic =
        readBytes(payload, offset, sizeof(KAD_ROUTING_STATE_MAGIC) - 1);
    if (magic != std::string(KAD_ROUTING_STATE_MAGIC,
                             sizeof(KAD_ROUTING_STATE_MAGIC) - 1)) {
      return false;
    }
    const auto version = readUInt32(readBytes(payload, offset, 4).data());
    if (version != KAD_ROUTING_STATE_VERSION) {
      return false;
    }
    KadRoutingSnapshot parsed;
    parsed.selfId = readBytes(payload, offset, HASH_LENGTH);
    parsed.lastBootstrap = readInt64(payload, offset);
    parsed.lastRefresh = readInt64(payload, offset);
    parsed.lastSelfRefresh = readInt64(payload, offset);
    const auto routerCount =
        readUInt16(readBytes(payload, offset, 2).data());
    parsed.routerNodes.reserve(routerCount);
    for (uint16_t i = 0; i < routerCount; ++i) {
      parsed.routerNodes.push_back(readStateEndpoint(payload, offset));
    }
    const auto bucketCount =
        readUInt16(readBytes(payload, offset, 2).data());
    parsed.buckets.reserve(bucketCount);
    for (uint16_t i = 0; i < bucketCount; ++i) {
      KadRoutingBucketSnapshot bucket;
      bucket.lastActive = readInt64(payload, offset);
      bucket.live = readKadRoutingNodes(payload, offset);
      bucket.replacements = readKadRoutingNodes(payload, offset);
      parsed.buckets.push_back(std::move(bucket));
    }
    if (offset != payload.size()) {
      return false;
    }
    snapshot = std::move(parsed);
    return true;
  }
  catch (DlAbortEx&) {
    return false;
  }
}

bool parseNodesDat(NodesDat& nodes, const std::string& payload)
{
  if (payload.size() < 4) {
    return false;
  }
  try {
    size_t offset = 0;
    uint32_t count = readUInt32(readBytes(payload, offset, 4).data());
    nodes.version = 0;
    nodes.bootstrapEdition = 0;
    nodes.contacts.clear();
    nodes.verified.clear();

    if (count == 0) {
      if (payload.size() - offset < 4) {
        return false;
      }
      nodes.version = readUInt32(readBytes(payload, offset, 4).data());
      if (nodes.version >= 1 && nodes.version <= 3) {
        if (nodes.version >= 3) {
          if (payload.size() - offset < 4) {
            return false;
          }
          nodes.bootstrapEdition =
              readUInt32(readBytes(payload, offset, 4).data());
        }
        if (payload.size() - offset < 4) {
          return false;
        }
        count = readUInt32(readBytes(payload, offset, 4).data());
      }
    }

    size_t entrySize = 25;
    const bool hasVerifiedData =
        nodes.version >= 2 && nodes.bootstrapEdition == 0;
    if (hasVerifiedData) {
      entrySize += 9;
    }
    if (payload.size() - offset < static_cast<size_t>(count) * entrySize) {
      return false;
    }

    nodes.contacts.reserve(count);
    if (hasVerifiedData) {
      nodes.verified.reserve(count);
    }
    for (uint32_t i = 0; i < count; ++i) {
      nodes.contacts.push_back(readKadContact(payload, offset));
      if (hasVerifiedData) {
        readBytes(payload, offset, 8);
        nodes.verified.push_back(readByte(payload, offset) != 0);
      }
    }
    return offset == payload.size();
  }
  catch (DlAbortEx&) {
    return false;
  }
}

std::string createFileStatusPayload(const std::string& fileHash,
                                    const std::vector<bool>& bitfield)
{
  validateHashLength(fileHash);
  if (bitfield.size() > std::numeric_limits<uint16_t>::max()) {
    throw DL_ABORT_EX("ED2K file status bitfield is too large.");
  }
  std::string payload = fileHash;
  payload += packUInt16(static_cast<uint16_t>(bitfield.size()));
  payload.append((bitfield.size() + 7) / 8, '\0');
  for (size_t i = 0; i < bitfield.size(); ++i) {
    if (bitfield[i]) {
      payload[HASH_LENGTH + 2 + i / 8] |= static_cast<char>(0x80 >> (i & 7));
    }
  }
  return payload;
}

bool parseFileStatusPayload(std::vector<bool>& bitfield,
                            const std::string& payload,
                            const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  size_t offset = 0;
  auto hash = readBytes(payload, offset, HASH_LENGTH);
  if (hash != expectedFileHash) {
    return false;
  }
  auto bitCount = readUInt16(readBytes(payload, offset, 2).data());
  auto byteCount = (bitCount + 7) / 8;
  auto raw = readBytes(payload, offset, byteCount);
  bitfield.assign(bitCount, false);
  for (size_t i = 0; i < bitCount; ++i) {
    bitfield[i] = raw[i / 8] & static_cast<char>(0x80 >> (i & 7));
  }
  return true;
}

std::string createHashSetAnswerPayload(
    const std::string& fileHash, const std::vector<std::string>& pieceHashes)
{
  validateHashLength(fileHash);
  if (pieceHashes.size() > std::numeric_limits<uint16_t>::max()) {
    throw DL_ABORT_EX("ED2K hash set is too large.");
  }
  std::string payload = fileHash;
  payload += packUInt16(static_cast<uint16_t>(pieceHashes.size()));
  for (const auto& hash : pieceHashes) {
    validateHashLength(hash);
    payload += hash;
  }
  return payload;
}

bool parseHashSetAnswerPayload(std::vector<std::string>& pieceHashes,
                               const std::string& payload,
                               const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  size_t offset = 0;
  auto hash = readBytes(payload, offset, HASH_LENGTH);
  if (hash != expectedFileHash) {
    return false;
  }
  auto count = readUInt16(readBytes(payload, offset, 2).data());
  pieceHashes.clear();
  pieceHashes.reserve(count);
  for (uint16_t i = 0; i < count; ++i) {
    pieceHashes.push_back(readBytes(payload, offset, HASH_LENGTH));
  }
  return true;
}

std::string createRequestPartsPayload(const std::string& fileHash,
                                      const std::vector<PartRange>& ranges,
                                      bool use64BitOffsets)
{
  validateHashLength(fileHash);
  if (ranges.size() > 3) {
    throw DL_ABORT_EX("Too many ED2K part ranges.");
  }
  std::string payload = fileHash;
  for (size_t i = 0; i < 3; ++i) {
    auto value = i < ranges.size() ? ranges[i].begin : 0;
    if (value < 0) {
      throw DL_ABORT_EX("Bad ED2K part range.");
    }
    payload += use64BitOffsets ? packUInt64(value) : packUInt32(value);
  }
  for (size_t i = 0; i < 3; ++i) {
    auto value = i < ranges.size() ? ranges[i].end : 0;
    if (value < 0 || (i < ranges.size() && value <= ranges[i].begin)) {
      throw DL_ABORT_EX("Bad ED2K part range.");
    }
    payload += use64BitOffsets ? packUInt64(value) : packUInt32(value);
  }
  return payload;
}

std::string createRequestSources2Payload(const std::string& fileHash)
{
  validateHashLength(fileHash);
  std::string payload;
  payload.push_back(static_cast<char>(SOURCE_EXCHANGE2_VERSION));
  payload += packUInt16(0);
  payload += fileHash;
  return payload;
}

bool parseRequestSources2Payload(uint8_t& version, const std::string& payload,
                                 const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  if (payload.size() == HASH_LENGTH) {
    version = 1;
    return payload == expectedFileHash;
  }
  if (payload.size() != HASH_LENGTH + 3) {
    return false;
  }
  size_t offset = 0;
  const auto parsedVersion = readByte(payload, offset);
  readUInt16(readBytes(payload, offset, 2).data());
  const auto hash = readBytes(payload, offset, HASH_LENGTH);
  if (parsedVersion == 0 || parsedVersion > SOURCE_EXCHANGE2_VERSION ||
      hash != expectedFileHash) {
    return false;
  }
  version = parsedVersion;
  return true;
}

std::string createAnswerSourcesPayload(
    const std::string& fileHash, uint8_t version,
    const std::vector<SourceExchangeEntry>& entries)
{
  validateHashLength(fileHash);
  if (version == 0 || version > SOURCE_EXCHANGE2_VERSION) {
    throw DL_ABORT_EX("Bad ED2K source exchange version.");
  }
  if (entries.size() > 500) {
    throw DL_ABORT_EX("Too many ED2K source exchange entries.");
  }
  std::string payload = fileHash;
  payload += packUInt16(static_cast<uint16_t>(entries.size()));
  for (const auto& entry : entries) {
    payload += packEndpoint(entry.endpoint);
    payload += packEndpoint(entry.server);
    if (version >= 2) {
      validateHashLength(entry.userHash);
      payload += entry.userHash;
    }
    if (version >= 4) {
      payload.push_back(static_cast<char>(entry.cryptOptions));
    }
  }
  return payload;
}

std::string createAnswerSources2Payload(
    const std::string& fileHash, uint8_t version,
    const std::vector<SourceExchangeEntry>& entries)
{
  validateHashLength(fileHash);
  if (version == 0 || version > SOURCE_EXCHANGE2_VERSION) {
    throw DL_ABORT_EX("Bad ED2K source exchange version.");
  }
  if (entries.size() > 500) {
    throw DL_ABORT_EX("Too many ED2K source exchange entries.");
  }
  std::string payload;
  payload.push_back(static_cast<char>(version));
  payload += fileHash;
  payload += packUInt16(static_cast<uint16_t>(entries.size()));
  for (const auto& entry : entries) {
    payload += packEndpoint(entry.endpoint);
    payload += packEndpoint(entry.server);
    if (version >= 2) {
      validateHashLength(entry.userHash);
      payload += entry.userHash;
    }
    if (version >= 4) {
      payload.push_back(static_cast<char>(entry.cryptOptions));
    }
  }
  return payload;
}

std::string createAnswerSources2Payload(
    const std::string& fileHash,
    const std::vector<SourceExchangeEntry>& entries)
{
  return createAnswerSources2Payload(fileHash, SOURCE_EXCHANGE2_VERSION,
                                     entries);
}

bool parseAnswerSourcesPayload(SourceExchangeAnswer& answer,
                               const std::string& payload,
                               const std::string& expectedFileHash,
                               uint8_t sourceExchangeVersion)
{
  validateHashLength(expectedFileHash);
  if (sourceExchangeVersion == 0 ||
      sourceExchangeVersion > SOURCE_EXCHANGE2_VERSION ||
      payload.size() < HASH_LENGTH + 2) {
    return false;
  }
  size_t offset = 0;
  const auto hash = readBytes(payload, offset, HASH_LENGTH);
  if (hash != expectedFileHash) {
    return false;
  }
  const auto count = readUInt16(readBytes(payload, offset, 2).data());
  if (count > 500) {
    return false;
  }
  const size_t entrySize = sourceExchangeVersion == 1   ? 12
                           : sourceExchangeVersion < 4 ? 28
                                                       : 29;
  if (payload.size() - offset != entrySize * count) {
    return false;
  }
  answer.version = sourceExchangeVersion;
  answer.entries.clear();
  answer.entries.reserve(count);
  for (uint16_t i = 0; i < count; ++i) {
    SourceExchangeEntry entry;
    entry.endpoint = readEndpoint(payload, offset);
    entry.server = readEndpoint(payload, offset);
    if (sourceExchangeVersion >= 2) {
      entry.userHash = readBytes(payload, offset, HASH_LENGTH);
      entry.endpoint.userHash = entry.userHash;
    }
    if (sourceExchangeVersion >= 4) {
      entry.cryptOptions = readByte(payload, offset);
      entry.endpoint.cryptOptions = entry.cryptOptions;
    }
    answer.entries.push_back(std::move(entry));
  }
  return true;
}

bool parseAnswerSources2Payload(SourceExchangeAnswer& answer,
                                const std::string& payload,
                                const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  size_t offset = 0;
  const auto version = readByte(payload, offset);
  const auto hash = readBytes(payload, offset, HASH_LENGTH);
  if (hash != expectedFileHash || version == 0 ||
      version > SOURCE_EXCHANGE2_VERSION) {
    return false;
  }
  const auto count = readUInt16(readBytes(payload, offset, 2).data());
  if (count > 500) {
    return false;
  }
  const size_t entrySize = version == 1   ? 12
                           : version < 4 ? 28
                                         : 29;
  if (payload.size() - offset != entrySize * count) {
    return false;
  }
  answer.version = version;
  answer.entries.clear();
  answer.entries.reserve(count);
  for (uint16_t i = 0; i < count; ++i) {
    SourceExchangeEntry entry;
    entry.endpoint = readEndpoint(payload, offset);
    entry.server = readEndpoint(payload, offset);
    if (version >= 2) {
      entry.userHash = readBytes(payload, offset, HASH_LENGTH);
    }
    if (version >= 4) {
      entry.cryptOptions = readByte(payload, offset);
      entry.endpoint.cryptOptions = entry.cryptOptions;
      entry.endpoint.userHash = entry.userHash;
    }
    answer.entries.push_back(std::move(entry));
  }
  return true;
}

bool parseCompressedPartPayload(CompressedPartHeader& header,
                                std::string& compressedData,
                                const std::string& payload,
                                const std::string& expectedFileHash,
                                bool use64BitOffsets)
{
  validateHashLength(expectedFileHash);
  const size_t metaLength = use64BitOffsets ? 28 : 24;
  if (payload.size() < metaLength ||
      payload.substr(0, HASH_LENGTH) != expectedFileHash) {
    return false;
  }
  const uint64_t begin = use64BitOffsets ? readUInt64(payload.data() + 16)
                                         : readUInt32(payload.data() + 16);
  const auto compressedLength =
      readUInt32(payload.data() + (use64BitOffsets ? 24 : 20));
  if (compressedLength != payload.size() - metaLength ||
      begin > static_cast<uint64_t>(std::numeric_limits<int64_t>::max())) {
    return false;
  }
  header.begin = static_cast<int64_t>(begin);
  header.compressedLength = compressedLength;
  compressedData.assign(payload.begin() + metaLength, payload.end());
  return true;
}

bool inflateCompressedPartData(std::string& inflatedData,
                               const std::string& compressedData,
                               size_t maxInflatedLength)
{
  z_stream strm;
  std::memset(&strm, 0, sizeof(strm));
  if (inflateInit(&strm) != Z_OK) {
    return false;
  }

  inflatedData.assign(maxInflatedLength, '\0');
  strm.avail_in = compressedData.size();
  strm.next_in = reinterpret_cast<unsigned char*>(
      const_cast<char*>(compressedData.data()));
  strm.avail_out = inflatedData.size();
  strm.next_out = reinterpret_cast<unsigned char*>(&inflatedData[0]);

  const int rc = inflate(&strm, Z_FINISH);
  const bool ok = rc == Z_STREAM_END && strm.avail_in == 0;
  const size_t produced = inflatedData.size() - strm.avail_out;
  inflateEnd(&strm);

  if (!ok || produced != maxInflatedLength) {
    inflatedData.clear();
    return false;
  }
  inflatedData.resize(produced);
  return true;
}

std::string createEmuleInfoPayload(const EmulePeerInfo& info)
{
  std::string payload;
  payload.push_back(static_cast<char>(info.version));
  payload.push_back(static_cast<char>(info.protocolVersion));
  payload += packUInt32(2);
  payload += createUInt32Tag(0xfb, emuleMiscOptionsValue(info.miscOptions));
  payload += createUInt32Tag(0xf6, emuleMiscOptions2Value(info.miscOptions2));
  return payload;
}

bool parseEmuleInfoPayload(EmulePeerInfo& info, const std::string& payload)
{
  if (payload.size() < 6) {
    return false;
  }
  info.version = static_cast<unsigned char>(payload[0]);
  info.protocolVersion = static_cast<unsigned char>(payload[1]);
  std::vector<Tag> tags;
  if (!parseTagList(tags, payload.substr(2))) {
    return false;
  }
  for (const auto& tag : tags) {
    if (tag.valueType != TagValueType::UINT) {
      continue;
    }
    if (tag.id == 0xfb) {
      info.miscOptions = parseEmuleMiscOptions(tag.intValue);
    }
    else if (tag.id == 0xf6) {
      info.miscOptions2 = parseEmuleMiscOptions2(tag.intValue);
    }
  }
  return true;
}

std::string createAichFileHashRequestPayload(const std::string& fileHash)
{
  validateHashLength(fileHash);
  return fileHash;
}

std::string createAichFileHashAnswerPayload(const std::string& fileHash,
                                            const std::string& rootHash)
{
  validateHashLength(fileHash);
  validateAichHashLength(rootHash);
  return fileHash + rootHash;
}

bool parseAichFileHashAnswerPayload(AichFileHashAnswer& answer,
                                    const std::string& payload,
                                    const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  if (payload.size() != HASH_LENGTH + AICH_HASH_LENGTH ||
      payload.substr(0, HASH_LENGTH) != expectedFileHash) {
    return false;
  }
  answer.fileHash = payload.substr(0, HASH_LENGTH);
  answer.rootHash = payload.substr(HASH_LENGTH, AICH_HASH_LENGTH);
  return true;
}

std::string createAichRequestPayload(const std::string& fileHash,
                                     uint16_t partIndex,
                                     const std::string& rootHash)
{
  validateHashLength(fileHash);
  validateAichHashLength(rootHash);
  return fileHash + packUInt16(partIndex) + rootHash;
}

bool parseAichRequestPayload(AichRequest& request, const std::string& payload,
                             const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  if (payload.size() != HASH_LENGTH + 2 + AICH_HASH_LENGTH ||
      payload.substr(0, HASH_LENGTH) != expectedFileHash) {
    return false;
  }
  request.fileHash = payload.substr(0, HASH_LENGTH);
  request.partIndex = readUInt16(payload.data() + HASH_LENGTH);
  request.rootHash = payload.substr(HASH_LENGTH + 2, AICH_HASH_LENGTH);
  return true;
}

std::string createAichAnswerPayload(const std::string& fileHash,
                                    uint16_t partIndex,
                                    const std::string& rootHash,
                                    const std::string& recoveryData)
{
  return createAichRequestPayload(fileHash, partIndex, rootHash) +
         recoveryData;
}

bool parseAichAnswerPayload(AichAnswer& answer, const std::string& payload,
                            const std::string& expectedFileHash)
{
  validateHashLength(expectedFileHash);
  if (payload.size() == HASH_LENGTH &&
      payload.substr(0, HASH_LENGTH) == expectedFileHash) {
    answer.failed = true;
    answer.fileHash = payload;
    answer.partIndex = 0;
    answer.rootHash.clear();
    answer.recoveryData.clear();
    return true;
  }
  if (payload.size() < HASH_LENGTH + 2 + AICH_HASH_LENGTH ||
      payload.substr(0, HASH_LENGTH) != expectedFileHash) {
    return false;
  }
  answer.failed = false;
  answer.fileHash = payload.substr(0, HASH_LENGTH);
  answer.partIndex = readUInt16(payload.data() + HASH_LENGTH);
  answer.rootHash = payload.substr(HASH_LENGTH + 2, AICH_HASH_LENGTH);
  answer.recoveryData.assign(payload.begin() + HASH_LENGTH + 2 +
                                 AICH_HASH_LENGTH,
                             payload.end());
  return true;
}

} // namespace ed2k

} // namespace aria2
