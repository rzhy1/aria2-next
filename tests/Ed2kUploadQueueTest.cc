#include "Ed2kUploadQueue.h"

#include <cppunit/extensions/HelperMacros.h>

#include "ed2k_hash.h"

namespace aria2 {

namespace ed2k {

class Ed2kUploadQueueTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Ed2kUploadQueueTest);
  CPPUNIT_TEST(testRejectsDuplicateUserHash);
  CPPUNIT_TEST(testCreditsSortWaitingPeers);
  CPPUNIT_TEST_SUITE_END();

public:
  void testRejectsDuplicateUserHash();
  void testCreditsSortWaitingPeers();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ed2kUploadQueueTest);

void Ed2kUploadQueueTest::testRejectsDuplicateUserHash()
{
  UploadQueue queue(1);
  Endpoint first;
  first.host = "203.0.113.10";
  first.port = 4662;
  Endpoint duplicate;
  duplicate.host = "203.0.113.11";
  duplicate.port = 4662;
  Endpoint other;
  other.host = "203.0.113.12";
  other.port = 4662;
  const std::string userHash(HASH_LENGTH, '\x44');
  const std::string otherUserHash(HASH_LENGTH, '\x45');
  const std::string fileHash(HASH_LENGTH, '\x66');

  CPPUNIT_ASSERT(queue.requestUpload(first, userHash, fileHash, 1000, nullptr));
  CPPUNIT_ASSERT(!queue.requestUpload(duplicate, userHash, fileHash, 1001,
                                      nullptr));
  CPPUNIT_ASSERT_EQUAL((size_t)1, queue.peers().size());
  CPPUNIT_ASSERT(queue.isUploading(first));

  CPPUNIT_ASSERT(!queue.requestUpload(other, otherUserHash, fileHash, 1002,
                                      nullptr));
  CPPUNIT_ASSERT_EQUAL((size_t)2, queue.peers().size());
  CPPUNIT_ASSERT_EQUAL((uint16_t)1, queue.queueRank(other));
}

void Ed2kUploadQueueTest::testCreditsSortWaitingPeers()
{
  UploadQueue queue(1);
  Endpoint active;
  active.host = "203.0.113.10";
  active.port = 4662;
  Endpoint older;
  older.host = "203.0.113.11";
  older.port = 4662;
  Endpoint credited;
  credited.host = "203.0.113.12";
  credited.port = 4662;
  const std::string activeHash(HASH_LENGTH, '\x40');
  const std::string olderHash(HASH_LENGTH, '\x41');
  const std::string creditedHash(HASH_LENGTH, '\x42');
  const std::string fileHash(HASH_LENGTH, '\x66');

  CPPUNIT_ASSERT(queue.requestUpload(active, activeHash, fileHash, 1000,
                                     nullptr));
  CPPUNIT_ASSERT(!queue.requestUpload(older, olderHash, fileHash, 1001,
                                      nullptr));
  queue.credits().addDownloaded(creditedHash, 4 * 1024 * 1024);
  CPPUNIT_ASSERT(!queue.requestUpload(credited, creditedHash, fileHash, 1002,
                                      nullptr));

  CPPUNIT_ASSERT_EQUAL((uint16_t)2, queue.queueRank(older));
  CPPUNIT_ASSERT_EQUAL((uint16_t)1, queue.queueRank(credited));
}

} // namespace ed2k

} // namespace aria2
