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
#include "Ed2kKadState.h"

#include <algorithm>
#include <limits>

#include "DlAbortEx.h"
#include "fmt.h"

namespace aria2 {

namespace ed2k {

namespace {

bool sameContact(const KadContact& lhs, const KadContact& rhs)
{
  return (!lhs.id.empty() && lhs.id == rhs.id) ||
         (lhs.host == rhs.host && lhs.udpPort == rhs.udpPort);
}

bool sameEndpoint(const Endpoint& lhs, const Endpoint& rhs)
{
  return lhs.host == rhs.host && lhs.port == rhs.port;
}

bool validId(const std::string& id) { return id.size() == HASH_LENGTH; }

void validateId(const std::string& id)
{
  if (!validId(id)) {
    throw DL_ABORT_EX("Bad ED2K Kad id length.");
  }
}

int distanceCompare(const std::string& lhs, const std::string& rhs,
                    const std::string& target)
{
  validateId(lhs);
  validateId(rhs);
  validateId(target);
  for (size_t i = 0; i < HASH_LENGTH; ++i) {
    const auto ld = static_cast<unsigned char>(lhs[i]) ^
                    static_cast<unsigned char>(target[i]);
    const auto rd = static_cast<unsigned char>(rhs[i]) ^
                    static_cast<unsigned char>(target[i]);
    if (ld < rd) {
      return -1;
    }
    if (ld > rd) {
      return 1;
    }
  }
  return 0;
}

std::vector<KadRoutingNode>::iterator findNode(
    std::vector<KadRoutingNode>& nodes, const KadContact& contact)
{
  return std::find_if(nodes.begin(), nodes.end(),
                      [&](const KadRoutingNode& node) {
                        return sameContact(node.contact, contact);
                      });
}

std::vector<KadRoutingNode>::const_iterator findNode(
    const std::vector<KadRoutingNode>& nodes, const KadContact& contact)
{
  return std::find_if(nodes.begin(), nodes.end(),
                      [&](const KadRoutingNode& node) {
                        return sameContact(node.contact, contact);
                      });
}

void updateNode(KadRoutingNode& node, const KadContact& contact, bool confirmed,
                int64_t now)
{
  node.contact = contact;
  node.lastSeen = now;
  if (node.firstSeen == 0) {
    node.firstSeen = now;
  }
  if (confirmed) {
    node.confirmed = true;
    node.failCount = 0;
  }
}

} // namespace

KadRoutingTable::KadRoutingTable(std::string selfId, size_t bucketSize)
    : selfId_(std::move(selfId)),
      bucketSize_(bucketSize == 0 ? 10 : bucketSize),
      buckets_(128)
{
  validateId(selfId_);
}

void KadRoutingTable::heardAbout(const KadContact& contact, int64_t now)
{
  addNode(contact, false, now);
}

void KadRoutingTable::nodeSeen(const KadContact& contact, int64_t now)
{
  addNode(contact, true, now);
}

void KadRoutingTable::addNode(const KadContact& contact, bool confirmed,
                              int64_t now)
{
  if (!validId(contact.id) || contact.host.empty() || contact.udpPort == 0 ||
      contact.id == selfId_) {
    return;
  }
  auto& bucket = buckets_[bucketIndex(contact.id)];
  bucket.lastActive = now;

  auto live = findNode(bucket.live, contact);
  if (live != bucket.live.end()) {
    updateNode(*live, contact, confirmed, now);
    auto node = *live;
    bucket.live.erase(live);
    bucket.live.push_back(node);
    return;
  }

  auto replacement = findNode(bucket.replacements, contact);
  if (replacement != bucket.replacements.end()) {
    updateNode(*replacement, contact, confirmed, now);
    if (confirmed && bucket.live.size() < bucketSize_) {
      auto node = *replacement;
      bucket.replacements.erase(replacement);
      bucket.live.push_back(node);
    }
    return;
  }

  KadRoutingNode node;
  node.contact = contact;
  node.confirmed = confirmed;
  node.firstSeen = now;
  node.lastSeen = now;
  if (bucket.live.size() < bucketSize_) {
    bucket.live.push_back(node);
    return;
  }
  if (bucket.replacements.size() == bucketSize_) {
    bucket.replacements.erase(bucket.replacements.begin());
  }
  bucket.replacements.push_back(node);
}

void KadRoutingTable::nodeFailed(const KadContact& contact)
{
  if (!validId(contact.id)) {
    return;
  }
  auto& bucket = buckets_[bucketIndex(contact.id)];
  auto live = findNode(bucket.live, contact);
  if (live != bucket.live.end()) {
    if (!live->confirmed) {
      bucket.live.erase(live);
      return;
    }
    ++live->failCount;
    if (!bucket.replacements.empty()) {
      *live = bucket.replacements.front();
      bucket.replacements.erase(bucket.replacements.begin());
      return;
    }
    if (live->failCount >= 20) {
      bucket.live.erase(live);
    }
    return;
  }
  auto replacement = findNode(bucket.replacements, contact);
  if (replacement != bucket.replacements.end()) {
    bucket.replacements.erase(replacement);
  }
}

std::vector<KadContact> KadRoutingTable::findClosest(
    const std::string& targetId, size_t limit, bool includeUnconfirmed) const
{
  validateId(targetId);
  std::vector<KadRoutingNode> nodes;
  for (const auto& bucket : buckets_) {
    for (const auto& node : bucket.live) {
      if (includeUnconfirmed || node.confirmed) {
        nodes.push_back(node);
      }
    }
  }
  std::sort(nodes.begin(), nodes.end(),
            [&](const KadRoutingNode& lhs, const KadRoutingNode& rhs) {
              return distanceCompare(lhs.contact.id, rhs.contact.id,
                                     targetId) < 0;
            });
  if (limit > 0 && nodes.size() > limit) {
    nodes.resize(limit);
  }
  std::vector<KadContact> contacts;
  contacts.reserve(nodes.size());
  for (const auto& node : nodes) {
    contacts.push_back(node.contact);
  }
  return contacts;
}

bool KadRoutingTable::needBootstrap(int64_t now)
{
  if (liveSize() != 0 || replacementSize() >= bucketSize_) {
    return false;
  }
  if (lastBootstrap_ == 0 || now - lastBootstrap_ >= 30) {
    lastBootstrap_ = now;
    return true;
  }
  return false;
}

bool KadRoutingTable::needRefresh(std::string& targetId, int64_t now)
{
  if (lastSelfRefresh_ == 0 || now - lastSelfRefresh_ >= 900) {
    lastSelfRefresh_ = now;
    targetId = selfId_;
    return true;
  }
  if (lastRefresh_ != 0 && now - lastRefresh_ < 45) {
    return false;
  }
  auto oldest = buckets_.end();
  for (auto i = buckets_.begin(); i != buckets_.end(); ++i) {
    if (i->lastActive == 0 || now - i->lastActive < 900) {
      continue;
    }
    if (oldest == buckets_.end() || i->lastActive < oldest->lastActive) {
      oldest = i;
    }
  }
  if (oldest == buckets_.end()) {
    return false;
  }
  lastRefresh_ = now;
  auto index = static_cast<size_t>(oldest - buckets_.begin());
  targetId = selfId_;
  auto byteIndex = index / 8;
  auto bitIndex = index % 8;
  targetId[byteIndex] =
      static_cast<char>(static_cast<unsigned char>(targetId[byteIndex]) ^
                        (0x80u >> bitIndex));
  return true;
}

void KadRoutingTable::addRouterNode(const Endpoint& endpoint)
{
  if (endpoint.host.empty() || endpoint.port == 0) {
    return;
  }
  if (std::find_if(routerNodes_.begin(), routerNodes_.end(),
                   [&](const Endpoint& item) {
                     return sameEndpoint(item, endpoint);
                   }) == routerNodes_.end()) {
    routerNodes_.push_back(endpoint);
  }
}

std::vector<Endpoint> KadRoutingTable::getRouterNodes() const
{
  return routerNodes_;
}

KadRoutingSnapshot KadRoutingTable::snapshot() const
{
  KadRoutingSnapshot snapshot;
  snapshot.selfId = selfId_;
  snapshot.lastBootstrap = lastBootstrap_;
  snapshot.lastRefresh = lastRefresh_;
  snapshot.lastSelfRefresh = lastSelfRefresh_;
  snapshot.routerNodes = routerNodes_;
  snapshot.buckets.reserve(buckets_.size());
  for (const auto& bucket : buckets_) {
    KadRoutingBucketSnapshot item;
    item.live = bucket.live;
    item.replacements = bucket.replacements;
    item.lastActive = bucket.lastActive;
    snapshot.buckets.push_back(std::move(item));
  }
  return snapshot;
}

void KadRoutingTable::restore(const KadRoutingSnapshot& snapshot)
{
  validateId(snapshot.selfId);
  selfId_ = snapshot.selfId;
  routerNodes_ = snapshot.routerNodes;
  lastBootstrap_ = snapshot.lastBootstrap;
  lastRefresh_ = snapshot.lastRefresh;
  lastSelfRefresh_ = snapshot.lastSelfRefresh;
  buckets_.assign(128, Bucket());
  const auto count = std::min(buckets_.size(), snapshot.buckets.size());
  for (size_t i = 0; i < count; ++i) {
    buckets_[i].live = snapshot.buckets[i].live;
    buckets_[i].replacements = snapshot.buckets[i].replacements;
    buckets_[i].lastActive = snapshot.buckets[i].lastActive;
  }
}

size_t KadRoutingTable::liveSize() const
{
  size_t size = 0;
  for (const auto& bucket : buckets_) {
    size += bucket.live.size();
  }
  return size;
}

size_t KadRoutingTable::replacementSize() const
{
  size_t size = 0;
  for (const auto& bucket : buckets_) {
    size += bucket.replacements.size();
  }
  return size;
}

size_t KadRoutingTable::bucketIndex(const std::string& id) const
{
  validateId(id);
  for (size_t i = 0; i < HASH_LENGTH; ++i) {
    const auto x = static_cast<unsigned char>(selfId_[i]) ^
                   static_cast<unsigned char>(id[i]);
    if (x == 0) {
      continue;
    }
    for (size_t bit = 0; bit < 8; ++bit) {
      if (x & (0x80u >> bit)) {
        return i * 8 + bit;
      }
    }
  }
  return 127;
}

void KadTransactionTable::add(const KadTransaction& transaction)
{
  transactions_.push_back(transaction);
}

bool KadTransactionTable::complete(const Endpoint& endpoint, uint8_t opcode,
                                   KadTransaction& transaction)
{
  auto i = std::find_if(transactions_.begin(), transactions_.end(),
                        [&](const KadTransaction& item) {
                          return item.expectedOpcode == opcode &&
                                 sameEndpoint(item.endpoint, endpoint);
                        });
  if (i == transactions_.end()) {
    return false;
  }
  transaction = *i;
  transactions_.erase(i);
  return true;
}

std::vector<KadTransaction> KadTransactionTable::expire(int64_t now,
                                                        int64_t timeoutSeconds)
{
  std::vector<KadTransaction> expired;
  auto i = transactions_.begin();
  while (i != transactions_.end()) {
    if (now - i->sentTime >= timeoutSeconds) {
      expired.push_back(*i);
      i = transactions_.erase(i);
    }
    else {
      ++i;
    }
  }
  return expired;
}

} // namespace ed2k

} // namespace aria2
