#include "Ed2kKadState.h"

#include <cppunit/extensions/HelperMacros.h>

#include "ed2k_constants.h"
#include "util.h"

namespace aria2 {

namespace ed2k {

class Ed2kKadStateTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Ed2kKadStateTest);
  CPPUNIT_TEST(testRoutingPromotesReplacementOnFailure);
  CPPUNIT_TEST(testRoutingFindClosestAndSnapshot);
  CPPUNIT_TEST(testRoutingBootstrapAndRefresh);
  CPPUNIT_TEST(testExpiredTransactionCarriesContactForFailure);
  CPPUNIT_TEST(testTransactionCompletionMatchesTarget);
  CPPUNIT_TEST(testTransactionCompletionAndExpiry);
  CPPUNIT_TEST_SUITE_END();

public:
  void testRoutingPromotesReplacementOnFailure();
  void testRoutingFindClosestAndSnapshot();
  void testRoutingBootstrapAndRefresh();
  void testExpiredTransactionCarriesContactForFailure();
  void testTransactionCompletionMatchesTarget();
  void testTransactionCompletionAndExpiry();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ed2kKadStateTest);

namespace {

std::string hashFromHex(const std::string& hex)
{
  return util::fromHex(hex.begin(), hex.end());
}

KadContact contactFromHex(const std::string& id, const std::string& host,
                          uint16_t udpPort)
{
  KadContact contact;
  contact.id = hashFromHex(id);
  contact.host = host;
  contact.udpPort = udpPort;
  contact.tcpPort = udpPort - 10;
  contact.version = 8;
  return contact;
}

Endpoint endpoint(const std::string& host, uint16_t port)
{
  Endpoint ep;
  ep.host = host;
  ep.port = port;
  return ep;
}

} // namespace

void Ed2kKadStateTest::testRoutingPromotesReplacementOnFailure()
{
  auto self = hashFromHex("23a8ceff57a7a32d562d649ed7893796");
  KadRoutingTable table(self, 1);
  auto live = contactFromHex("31d6cfe0d16ae931b73c59d7e0c089c0",
                             "1.2.3.4", 4672);
  auto replacement = contactFromHex("31d6cfe0d14ce931b73c59d7e0c04bc0",
                                    "5.6.7.8", 4672);

  table.nodeSeen(live, 10);
  table.heardAbout(replacement, 11);
  table.nodeFailed(live);

  auto closest = table.findClosest(self, 10, true);
  CPPUNIT_ASSERT_EQUAL((size_t)1, closest.size());
  CPPUNIT_ASSERT_EQUAL(replacement.id, closest[0].id);
  CPPUNIT_ASSERT_EQUAL((size_t)1, table.liveSize());
  CPPUNIT_ASSERT_EQUAL((size_t)0, table.replacementSize());
}

void Ed2kKadStateTest::testRoutingFindClosestAndSnapshot()
{
  auto self = hashFromHex("00000000000000000000000000000000");
  KadRoutingTable table(self, 10);
  auto near = contactFromHex("00000000000000000000000000000001",
                             "1.2.3.4", 4672);
  auto far = contactFromHex("80000000000000000000000000000000",
                            "5.6.7.8", 4672);
  auto unconfirmed = contactFromHex("00000000000000000000000000000002",
                                    "9.9.9.9", 4672);
  table.nodeSeen(far, 10);
  table.nodeSeen(near, 11);
  table.heardAbout(unconfirmed, 12);
  table.addRouterNode(endpoint("203.0.113.1", 4672));
  table.addRouterNode(endpoint("203.0.113.1", 4672));

  auto confirmed = table.findClosest(self, 10, false);
  CPPUNIT_ASSERT_EQUAL((size_t)2, confirmed.size());
  CPPUNIT_ASSERT_EQUAL(near.id, confirmed[0].id);
  CPPUNIT_ASSERT_EQUAL(far.id, confirmed[1].id);

  auto all = table.findClosest(self, 10, true);
  CPPUNIT_ASSERT_EQUAL((size_t)3, all.size());
  CPPUNIT_ASSERT_EQUAL((size_t)1, table.getRouterNodes().size());

  KadRoutingTable restored(self, 10);
  restored.restore(table.snapshot());
  CPPUNIT_ASSERT_EQUAL((size_t)3, restored.liveSize());
  CPPUNIT_ASSERT_EQUAL((size_t)1, restored.getRouterNodes().size());
}

void Ed2kKadStateTest::testRoutingBootstrapAndRefresh()
{
  auto self = hashFromHex("00000000000000000000000000000000");
  KadRoutingTable table(self, 10);
  CPPUNIT_ASSERT(table.needBootstrap(100));
  CPPUNIT_ASSERT(!table.needBootstrap(120));
  CPPUNIT_ASSERT(table.needBootstrap(131));

  std::string target;
  CPPUNIT_ASSERT(table.needRefresh(target, 200));
  CPPUNIT_ASSERT_EQUAL(self, target);
  CPPUNIT_ASSERT(!table.needRefresh(target, 210));
}

void Ed2kKadStateTest::testExpiredTransactionCarriesContactForFailure()
{
  KadTransactionTable table;
  KadTransaction tx;
  tx.endpoint = endpoint("203.0.113.9", 4672);
  tx.contact = contactFromHex("31d6cfe0d16ae931b73c59d7e0c089c0",
                              "203.0.113.9", 4672);
  tx.expectedOpcode = KAD_RES;
  tx.targetId = hashFromHex("00000000000000000000000000000000");
  tx.sentTime = 100;
  table.add(tx);

  auto expired = table.expire(113, 12);
  CPPUNIT_ASSERT_EQUAL((size_t)1, expired.size());
  CPPUNIT_ASSERT_EQUAL(tx.contact.id, expired[0].contact.id);
  CPPUNIT_ASSERT_EQUAL(tx.contact.host, expired[0].contact.host);
  CPPUNIT_ASSERT_EQUAL(tx.contact.udpPort, expired[0].contact.udpPort);
}

void Ed2kKadStateTest::testTransactionCompletionMatchesTarget()
{
  KadTransactionTable table;
  KadTransaction refresh;
  refresh.endpoint = endpoint("203.0.113.9", 4672);
  refresh.expectedOpcode = KAD_RES;
  refresh.purpose = KadTransactionPurpose::REFRESH;
  refresh.targetId = hashFromHex("ffffffffffffffffffffffffffffffff");
  refresh.sentTime = 100;
  table.add(refresh);

  KadTransaction lookup;
  lookup.endpoint = refresh.endpoint;
  lookup.expectedOpcode = KAD_RES;
  lookup.purpose = KadTransactionPurpose::SOURCE_LOOKUP;
  lookup.targetId = hashFromHex("0123456789abcdef0123456789abcdef");
  lookup.sentTime = 100;
  table.add(lookup);

  KadTransaction completed;
  CPPUNIT_ASSERT(table.complete(endpoint("203.0.113.9", 4672), KAD_RES,
                                lookup.targetId, completed));
  CPPUNIT_ASSERT_EQUAL(KadTransactionPurpose::SOURCE_LOOKUP,
                       completed.purpose);
  CPPUNIT_ASSERT_EQUAL(lookup.targetId, completed.targetId);
  CPPUNIT_ASSERT_EQUAL((size_t)1, table.size());
}

void Ed2kKadStateTest::testTransactionCompletionAndExpiry()
{
  KadTransactionTable table;
  KadTransaction tx;
  tx.endpoint = endpoint("203.0.113.9", 4672);
  tx.expectedOpcode = KAD_BOOTSTRAP_RES;
  tx.targetId = hashFromHex("00000000000000000000000000000000");
  tx.sentTime = 100;
  table.add(tx);

  KadTransaction completed;
  CPPUNIT_ASSERT(table.complete(endpoint("203.0.113.9", 4672),
                                KAD_BOOTSTRAP_RES, completed));
  CPPUNIT_ASSERT_EQUAL(tx.targetId, completed.targetId);
  CPPUNIT_ASSERT_EQUAL((size_t)0, table.size());

  table.add(tx);
  auto expired = table.expire(113, 12);
  CPPUNIT_ASSERT_EQUAL((size_t)1, expired.size());
  CPPUNIT_ASSERT_EQUAL((size_t)0, table.size());
}

} // namespace ed2k

} // namespace aria2
