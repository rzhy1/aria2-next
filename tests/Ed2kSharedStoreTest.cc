#include "Ed2kSharedStore.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadResult.h"
#include "Ed2kAttribute.h"
#include "File.h"
#include "FileEntry.h"
#include "TestUtil.h"
#include "ed2k_link.h"

namespace aria2 {

namespace ed2k {

class Ed2kSharedStoreTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(Ed2kSharedStoreTest);
  CPPUNIT_TEST(testAddCompletedDownload);
  CPPUNIT_TEST(testRejectMissingOrWrongSizeFile);
  CPPUNIT_TEST_SUITE_END();

public:
  void testAddCompletedDownload();
  void testRejectMissingOrWrongSizeFile();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Ed2kSharedStoreTest);

namespace {

std::shared_ptr<DownloadResult> createEd2kResult(const std::string& path,
                                                 int64_t size,
                                                 const std::string& hash)
{
  auto result = std::make_shared<DownloadResult>();
  result->result = error_code::FINISHED;
  result->totalLength = size;
  result->fileEntries.push_back(std::make_shared<FileEntry>(path, size, 0));
  auto attrs = std::make_shared<Ed2kAttribute>();
  attrs->link.type = LinkType::FILE;
  attrs->link.name = "shared.bin";
  attrs->link.size = size;
  attrs->link.hash = hash;
  attrs->pieceHashes.push_back(hash);
  attrs->aichRootHash.assign(AICH_HASH_LENGTH, '\x22');
  result->attrs.push_back(attrs);
  return result;
}

} // namespace

void Ed2kSharedStoreTest::testAddCompletedDownload()
{
  const std::string hash(16, '\x11');
  const std::string path = A2_TEST_OUT_DIR "/ed2k-shared-store-completed.bin";
  createFile(path, 123);

  SharedStore store;
  CPPUNIT_ASSERT(store.addCompletedDownload(*createEd2kResult(path, 123, hash),
                                            1000));

  auto file = store.findByHash(hash);
  CPPUNIT_ASSERT(file);
  CPPUNIT_ASSERT_EQUAL(std::string("shared.bin"), file->name);
  CPPUNIT_ASSERT_EQUAL(path, file->path);
  CPPUNIT_ASSERT_EQUAL((int64_t)123, file->size);
  CPPUNIT_ASSERT_EQUAL((size_t)1, file->pieceHashes.size());
  CPPUNIT_ASSERT_EQUAL((int64_t)1000, file->lastHashTime);
  CPPUNIT_ASSERT(file->completed);
  CPPUNIT_ASSERT(file->origin == SharedOrigin::COMPLETED_DOWNLOAD);

  const std::string updatedPath =
      A2_TEST_OUT_DIR "/ed2k-shared-store-updated.bin";
  createFile(updatedPath, 123);
  CPPUNIT_ASSERT(store.addCompletedDownload(
      *createEd2kResult(updatedPath, 123, hash), 2000));
  CPPUNIT_ASSERT_EQUAL((size_t)1, store.size());
  file = store.findByHash(hash);
  CPPUNIT_ASSERT(file);
  CPPUNIT_ASSERT_EQUAL(updatedPath, file->path);
  CPPUNIT_ASSERT_EQUAL((int64_t)2000, file->lastHashTime);
}

void Ed2kSharedStoreTest::testRejectMissingOrWrongSizeFile()
{
  const std::string hash(16, '\x33');
  const std::string missingPath =
      A2_TEST_OUT_DIR "/ed2k-shared-store-missing.bin";
  File(missingPath).remove();

  SharedStore store;
  CPPUNIT_ASSERT(!store.addCompletedDownload(
      *createEd2kResult(missingPath, 123, hash), 1000));
  CPPUNIT_ASSERT(!store.findByHash(hash));

  const std::string wrongSizePath =
      A2_TEST_OUT_DIR "/ed2k-shared-store-wrong-size.bin";
  createFile(wrongSizePath, 122);
  CPPUNIT_ASSERT(!store.addCompletedDownload(
      *createEd2kResult(wrongSizePath, 123, hash), 1000));
  CPPUNIT_ASSERT_EQUAL((size_t)0, store.size());
}

} // namespace ed2k

} // namespace aria2
