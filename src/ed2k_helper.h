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
#ifndef D_ED2K_HELPER_H
#define D_ED2K_HELPER_H

#include "common.h"
#include "ed2k_hash.h"
#include "ed2k_link.h"

#include <cstdint>
#include <string>
#include <vector>

namespace aria2 {

namespace ed2k {

struct KadRoutingSnapshot;

constexpr uint8_t PROTO_EDONKEY = 0xe3;
constexpr uint8_t PROTO_EMULE = 0xc5;
constexpr uint8_t OP_LOGINREQUEST = 0x01;
constexpr uint8_t OP_GETSERVERLIST = 0x14;
constexpr uint8_t OP_SEARCHREQUEST = 0x16;
constexpr uint8_t OP_GETSOURCES = 0x19;
constexpr uint8_t OP_CALLBACKREQUEST = 0x1c;
constexpr uint8_t OP_QUERY_MORE_RESULT = 0x21;
constexpr uint8_t OP_SERVERLIST = 0x32;
constexpr uint8_t OP_SEARCHRESULT = 0x33;
constexpr uint8_t OP_SERVERSTATUS = 0x34;
constexpr uint8_t OP_CALLBACKREQUESTED = 0x35;
constexpr uint8_t OP_CALLBACK_FAIL = 0x36;
constexpr uint8_t OP_SERVERMESSAGE = 0x38;
constexpr uint8_t OP_IDCHANGE = 0x40;
constexpr uint8_t OP_FOUNDSOURCES = 0x42;
constexpr uint8_t OP_GLOBSERVSTATREQ = 0x96;
constexpr uint8_t OP_GLOBSERVSTATRES = 0x97;
constexpr uint8_t OP_HELLO = 0x01;
constexpr uint8_t OP_HELLOANSWER = 0x4c;
constexpr uint8_t OP_SETREQFILEID = 0x4f;
constexpr uint8_t OP_FILESTATUS = 0x50;
constexpr uint8_t OP_HASHSETREQUEST = 0x51;
constexpr uint8_t OP_HASHSETANSWER = 0x52;
constexpr uint8_t OP_STARTUPLOADREQ = 0x54;
constexpr uint8_t OP_ACCEPTUPLOADREQ = 0x55;
constexpr uint8_t OP_FILEREQANSNOFIL = 0x48;
constexpr uint8_t OP_OUTOFPARTREQS = 0x57;
constexpr uint8_t OP_QUEUERANK = 0x5c;
constexpr uint8_t OP_REQUESTFILENAME = 0x58;
constexpr uint8_t OP_REQFILENAMEANSWER = 0x59;
constexpr uint8_t OP_REQUESTPARTS = 0x47;
constexpr uint8_t OP_SENDINGPART = 0x46;
constexpr uint8_t OP_COMPRESSEDPART = 0x40;
constexpr uint8_t OP_EMULEINFO = 0x01;
constexpr uint8_t OP_EMULEINFOANSWER = 0x02;
constexpr uint8_t OP_REQUESTSOURCES = 0x81;
constexpr uint8_t OP_ANSWERSOURCES = 0x82;
constexpr uint8_t OP_REQUESTSOURCES2 = 0x83;
constexpr uint8_t OP_ANSWERSOURCES2 = 0x84;
constexpr uint8_t OP_AICHREQUEST = 0x9b;
constexpr uint8_t OP_AICHANSWER = 0x9c;
constexpr uint8_t OP_AICHFILEHASHANS = 0x9d;
constexpr uint8_t OP_AICHFILEHASHREQ = 0x9e;
constexpr uint8_t OP_COMPRESSEDPART_I64 = 0xa1;
constexpr uint8_t OP_REQUESTPARTS_I64 = 0xa3;
constexpr uint8_t OP_SENDINGPART_I64 = 0xa2;
constexpr uint8_t SOURCE_EXCHANGE2_VERSION = 4;
constexpr uint8_t KAD_PROTOCOL = 0xe4;
constexpr uint8_t KAD_BOOTSTRAP_REQ = 0x01;
constexpr uint8_t KAD_BOOTSTRAP_RES = 0x09;
constexpr uint8_t KAD_HELLO_REQ = 0x11;
constexpr uint8_t KAD_HELLO_RES = 0x19;
constexpr uint8_t KAD_REQ = 0x21;
constexpr uint8_t KAD_RES = 0x29;
constexpr uint8_t KAD_SEARCH_KEYS_REQ = 0x33;
constexpr uint8_t KAD_SEARCH_SOURCES_REQ = 0x34;
constexpr uint8_t KAD_SEARCH_RES = 0x3b;
constexpr uint8_t KAD_PUBLISH_SOURCE_REQ = 0x44;
constexpr uint8_t KAD_PUBLISH_RES = 0x4b;
constexpr uint8_t KAD_FIREWALLED_REQ = 0x53;
constexpr uint8_t KAD_FIREWALLED_RES = 0x58;
constexpr uint8_t KAD_PING = 0x60;
constexpr uint8_t KAD_PONG = 0x61;
constexpr uint8_t KAD_FIREWALLED_UDP = 0x62;
constexpr uint8_t KAD_FIND_VALUE = 0x02;
constexpr uint8_t KAD_STORE = 0x04;
constexpr uint8_t KAD_FIND_NODE = 0x0b;

struct PacketHeader {
  uint8_t protocol = 0;
  uint32_t size = 0;
  uint8_t opcode = 0;

  size_t payloadSize() const { return size == 0 ? 0 : size - 1; }
};

enum class TagValueType {
  UNKNOWN,
  STRING,
  UINT,
  HASH,
  BLOB,
};

struct Tag {
  uint8_t id = 0;
  std::string name;
  uint8_t rawType = 0;
  TagValueType valueType = TagValueType::UNKNOWN;
  std::string stringValue;
  uint64_t intValue = 0;
  std::string binaryValue;
};

struct PartRange {
  int64_t begin = 0;
  int64_t end = 0;
};

struct SourceExchangeEntry {
  Endpoint endpoint;
  Endpoint server;
  std::string userHash;
  uint8_t cryptOptions = 0;
};

struct SourceExchangeAnswer {
  uint8_t version = 0;
  std::vector<SourceExchangeEntry> entries;
};

struct CompressedPartHeader {
  int64_t begin = 0;
  uint32_t compressedLength = 0;
};

struct EmuleMiscOptions {
  uint8_t aichVersion = 0;
  bool unicode = false;
  uint8_t udpVersion = 0;
  uint8_t dataCompressionVersion = 0;
  uint8_t secureIdentVersion = 0;
  uint8_t sourceExchange1Version = 0;
  uint8_t extendedRequestsVersion = 0;
  bool multiPacket = false;
};

struct EmuleMiscOptions2 {
  bool supportsLargeFiles = false;
  bool supportsExtendedMultipacket = false;
  bool supportsSourceExchange2 = false;
};

struct EmulePeerInfo {
  uint8_t version = 0;
  uint8_t protocolVersion = 0;
  EmuleMiscOptions miscOptions;
  EmuleMiscOptions2 miscOptions2;
};

struct AichFileHashAnswer {
  std::string fileHash;
  std::string rootHash;
};

struct AichRequest {
  std::string fileHash;
  uint16_t partIndex = 0;
  std::string rootHash;
};

struct AichAnswer {
  bool failed = false;
  std::string fileHash;
  uint16_t partIndex = 0;
  std::string rootHash;
  std::string recoveryData;
};

struct ServerIdChange {
  uint32_t clientId = 0;
  uint32_t tcpFlags = 0;
  uint32_t auxPort = 0;
  bool highId = false;
  std::string ipAddress;
};

struct ServerStatus {
  uint32_t challenge = 0;
  uint32_t users = 0;
  uint32_t files = 0;
  uint32_t maxUsers = 0;
  uint32_t softFiles = 0;
  uint32_t hardFiles = 0;
  uint32_t udpFlags = 0;
  uint32_t lowIdUsers = 0;
  uint16_t udpObfuscationPort = 0;
  uint16_t tcpObfuscationPort = 0;
  uint32_t udpKey = 0;
};

struct ServerState {
  Endpoint endpoint;
  bool connected = false;
  bool handshakeCompleted = false;
  uint32_t clientId = 0;
  bool highId = false;
  std::string ipAddress;
  uint32_t tcpFlags = 0;
  uint32_t users = 0;
  uint32_t files = 0;
  uint32_t maxUsers = 0;
  uint32_t softFiles = 0;
  uint32_t hardFiles = 0;
  uint32_t udpFlags = 0;
  uint32_t lowIdUsers = 0;
  uint16_t udpObfuscationPort = 0;
  uint16_t tcpObfuscationPort = 0;
  uint32_t udpKey = 0;
  uint32_t udpStatusChallenge = 0;
  int64_t lastUdpStatusTime = 0;
  uint32_t failCount = 0;
  int64_t lastFailureTime = 0;
  int64_t nextRetryTime = 0;
  std::string lastMessage;
};

struct SearchQuery {
  std::string keyword;
  std::string fileType;
  std::string extension;
  int64_t minSize = 0;
  int64_t maxSize = 0;
  uint32_t minSourceCount = 0;
  uint32_t minCompleteSourceCount = 0;
};

struct SearchResultEntry {
  std::string hash;
  std::string name;
  int64_t size = 0;
  uint32_t sourceCount = 0;
  uint32_t completeSourceCount = 0;
  std::string fileType;
  std::string extension;
  std::string mediaArtist;
  std::string mediaAlbum;
  std::string mediaTitle;
  std::string mediaLength;
  uint32_t mediaBitrate = 0;
  std::string mediaCodec;
  std::string sourceNetwork;
  std::string ed2kLink;
  std::vector<Tag> tags;
};

struct SearchResult {
  std::vector<SearchResultEntry> entries;
  bool moreResults = false;
};

struct KadContact {
  std::string id;
  std::string host;
  uint16_t udpPort = 0;
  uint16_t tcpPort = 0;
  uint8_t version = 0;
};

struct KadHello {
  std::string id;
  uint16_t tcpPort = 0;
  uint8_t version = 0;
  std::vector<Tag> tags;
};

struct KadBootstrapResponse {
  std::string id;
  uint16_t tcpPort = 0;
  uint8_t version = 0;
  std::vector<KadContact> contacts;
};

struct KadRequest {
  uint8_t searchType = 0;
  std::string targetId;
  std::string receiverId;
};

struct KadResponse {
  std::string targetId;
  std::vector<KadContact> contacts;
};

struct KadSearchEntry {
  std::string id;
  std::vector<Tag> tags;
};

struct KadSearchSourcesRequest {
  std::string targetId;
  uint16_t startPosition = 0;
  uint64_t size = 0;
};

struct KadSearchResult {
  std::string sourceId;
  std::string targetId;
  std::vector<KadSearchEntry> entries;
};

struct KadPublishSourceRequest {
  std::string fileId;
  KadSearchEntry source;
};

struct KadPublishResult {
  std::string fileId;
  uint8_t count = 0;
};

struct KadFirewalledRequest {
  uint16_t tcpPort = 0;
  std::string id;
  uint8_t options = 0;
};

struct KadFirewalledResponse {
  std::string ipAddress;
};

struct KadFirewalledUdp {
  uint8_t errorCode = 0;
  uint16_t tcpPort = 0;
};

struct NodesDat {
  uint32_t version = 0;
  uint32_t bootstrapEdition = 0;
  std::vector<KadContact> contacts;
  std::vector<bool> verified;
};

uint16_t readUInt16(const char* data);
uint32_t readUInt32(const char* data);
uint64_t readUInt64(const char* data);
std::string packUInt16(uint16_t value);
std::string packUInt32(uint32_t value);
std::string packUInt64(uint64_t value);
std::string createPacket(uint8_t protocol, uint8_t opcode,
                         const std::string& payload);
bool readPacketHeader(PacketHeader& header, const char* data, size_t length);
bool parseTagList(std::vector<Tag>& tags, const std::string& payload);
std::vector<Endpoint> parseServerMet(const std::string& data);
std::string createLoginRequestPayload(const std::string& clientHash,
                                      uint16_t listenPort,
                                      const std::string& clientName);
std::string createGetSourcesPayload(const std::string& fileHash,
                                    int64_t fileSize);
std::string createFoundSourcesPayload(const std::string& fileHash,
                                      const std::vector<Endpoint>& sources);
std::vector<Endpoint> parseFoundSourcesPayload(const std::string& payload);
bool parseFoundSourcesPayload(std::vector<Endpoint>& sources,
                              const std::string& payload,
                              const std::string& expectedFileHash);
bool parseFoundSourcesPayload(std::vector<FoundSource>& sources,
                              const std::string& payload,
                              const std::string& expectedFileHash);
std::string createCallbackRequestPayload(uint32_t clientId);
bool parseCallbackRequestIncomingPayload(Endpoint& endpoint,
                                         const std::string& payload);
bool parseServerIdChangePayload(ServerIdChange& idChange,
                                const std::string& payload);
bool parseServerStatusPayload(ServerStatus& status,
                              const std::string& payload);
bool parseServerMessagePayload(std::string& message,
                               const std::string& payload);
bool parseServerListPayload(std::vector<Endpoint>& servers,
                            const std::string& payload);
std::string createServerStatePayload(const ServerState& state);
bool parseServerStatePayload(ServerState& state, const std::string& payload);
bool parseSearchResultPayload(SearchResult& result, const std::string& payload,
                              const std::string& sourceNetwork);
std::string createSearchRequestPayload(const SearchQuery& query,
                                       bool supports64Bit);
std::string pickKadKeyword(const std::string& query);
std::string createKadKeywordTarget(const std::string& query);
std::string createKadHelloPayload(const std::string& id, uint16_t tcpPort,
                                  uint8_t version);
bool parseKadHelloPayload(KadHello& hello, const std::string& payload);
std::string createKadBootstrapResponsePayload(
    const std::string& id, uint16_t tcpPort, uint8_t version,
    const std::vector<KadContact>& contacts);
bool parseKadBootstrapResponsePayload(KadBootstrapResponse& response,
                                      const std::string& payload);
std::string createKadRequestPayload(uint8_t searchType,
                                    const std::string& targetId,
                                    const std::string& receiverId);
bool parseKadRequestPayload(KadRequest& request, const std::string& payload);
std::string createKadResponsePayload(const std::string& targetId,
                                     const std::vector<KadContact>& contacts);
bool parseKadResponsePayload(KadResponse& response,
                             const std::string& payload);
std::string createKadSearchSourcesRequestPayload(const std::string& targetId,
                                                 uint16_t startPosition,
                                                 uint64_t size);
bool parseKadSearchSourcesRequestPayload(KadSearchSourcesRequest& request,
                                         const std::string& payload);
std::string createKadSearchKeysRequestPayload(const std::string& targetId,
                                              uint16_t startPosition);
bool parseKadSearchResultPayload(KadSearchResult& result,
                                 const std::string& payload);
std::vector<Endpoint> extractKadSourceEndpoints(const KadSearchResult& result);
std::vector<SearchResultEntry> kadSearchEntriesToSearchResults(
    const std::vector<KadSearchEntry>& entries,
    const std::string& sourceNetwork);
std::string createKadPublishSourceRequestPayload(const std::string& fileId,
                                                 const Endpoint& source,
                                                 const std::string& sourceId);
bool parseKadPublishSourceRequestPayload(KadPublishSourceRequest& request,
                                         const std::string& payload);
std::string createKadPublishResultPayload(const std::string& fileId,
                                          uint8_t count);
bool parseKadPublishResultPayload(KadPublishResult& result,
                                  const std::string& payload);
std::string createKadFirewalledRequestPayload(uint16_t tcpPort,
                                              const std::string& id,
                                              uint8_t options);
bool parseKadFirewalledRequestPayload(KadFirewalledRequest& request,
                                      const std::string& payload);
std::string createKadFirewalledResponsePayload(const std::string& ipAddress);
bool parseKadFirewalledResponsePayload(KadFirewalledResponse& response,
                                       const std::string& payload);
std::string createKadFirewalledUdpPayload(uint8_t errorCode,
                                          uint16_t tcpPort);
bool parseKadFirewalledUdpPayload(KadFirewalledUdp& response,
                                  const std::string& payload);
std::string createKadRoutingStatePayload(const KadRoutingSnapshot& snapshot);
bool parseKadRoutingStatePayload(KadRoutingSnapshot& snapshot,
                                 const std::string& payload);
bool parseNodesDat(NodesDat& nodes, const std::string& payload);
std::string createFileStatusPayload(const std::string& fileHash,
                                    const std::vector<bool>& bitfield);
bool parseFileStatusPayload(std::vector<bool>& bitfield,
                            const std::string& payload,
                            const std::string& expectedFileHash);
std::string createHashSetAnswerPayload(
    const std::string& fileHash, const std::vector<std::string>& pieceHashes);
bool parseHashSetAnswerPayload(std::vector<std::string>& pieceHashes,
                               const std::string& payload,
                               const std::string& expectedFileHash);
std::string createRequestPartsPayload(const std::string& fileHash,
                                      const std::vector<PartRange>& ranges,
                                      bool use64BitOffsets);
std::string createRequestSources2Payload(const std::string& fileHash);
bool parseRequestSources2Payload(uint8_t& version, const std::string& payload,
                                 const std::string& expectedFileHash);
std::string createAnswerSourcesPayload(
    const std::string& fileHash, uint8_t version,
    const std::vector<SourceExchangeEntry>& entries);
bool parseAnswerSourcesPayload(SourceExchangeAnswer& answer,
                               const std::string& payload,
                               const std::string& expectedFileHash,
                               uint8_t sourceExchangeVersion);
std::string createAnswerSources2Payload(
    const std::string& fileHash, uint8_t version,
    const std::vector<SourceExchangeEntry>& entries);
std::string createAnswerSources2Payload(
    const std::string& fileHash,
    const std::vector<SourceExchangeEntry>& entries);
bool parseAnswerSources2Payload(SourceExchangeAnswer& answer,
                                const std::string& payload,
                                const std::string& expectedFileHash);
bool parseCompressedPartPayload(CompressedPartHeader& header,
                                std::string& compressedData,
                                const std::string& payload,
                                const std::string& expectedFileHash,
                                bool use64BitOffsets);
bool inflateCompressedPartData(std::string& inflatedData,
                               const std::string& compressedData,
                               size_t maxInflatedLength);
std::string createEmuleInfoPayload(const EmulePeerInfo& info);
bool parseEmuleInfoPayload(EmulePeerInfo& info, const std::string& payload);
std::string createAichFileHashRequestPayload(const std::string& fileHash);
std::string createAichFileHashAnswerPayload(const std::string& fileHash,
                                            const std::string& rootHash);
bool parseAichFileHashAnswerPayload(AichFileHashAnswer& answer,
                                    const std::string& payload,
                                    const std::string& expectedFileHash);
std::string createAichRequestPayload(const std::string& fileHash,
                                     uint16_t partIndex,
                                     const std::string& rootHash);
bool parseAichRequestPayload(AichRequest& request, const std::string& payload,
                             const std::string& expectedFileHash);
std::string createAichAnswerPayload(const std::string& fileHash,
                                    uint16_t partIndex,
                                    const std::string& rootHash,
                                    const std::string& recoveryData);
bool parseAichAnswerPayload(AichAnswer& answer, const std::string& payload,
                            const std::string& expectedFileHash);

} // namespace ed2k

} // namespace aria2

#endif // D_ED2K_HELPER_H
