#include "RotatingLogFile.h"

#include <cppunit/extensions/HelperMacros.h>

#include "BufferedFile.h"
#include "File.h"

namespace aria2 {

class RotatingLogFileTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RotatingLogFileTest);
  CPPUNIT_TEST(testRotationKeepsEachFileBounded);
  CPPUNIT_TEST(testRetentionLimitsTotalFiles);
  CPPUNIT_TEST(testOpenConvergesLegacyLogs);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testRotationKeepsEachFileBounded();
  void testRetentionLimitsTotalFiles();
  void testOpenConvergesLegacyLogs();

private:
  std::string path_;

  void removeLogs();
  void writeFile(const std::string& path, size_t size);
};

CPPUNIT_TEST_SUITE_REGISTRATION(RotatingLogFileTest);

void RotatingLogFileTest::setUp()
{
  path_ = A2_TEST_OUT_DIR "/aria2_RotatingLogFileTest.log";
  removeLogs();
}

void RotatingLogFileTest::tearDown() { removeLogs(); }

void RotatingLogFileTest::removeLogs()
{
  File(path_).remove();
  for (size_t i = 1; i <= 8; ++i) {
    File(path_ + "." + std::to_string(i)).remove();
  }
}

void RotatingLogFileTest::writeFile(const std::string& path, size_t size)
{
  BufferedFile file(path.c_str(), BufferedFile::WRITE);
  CPPUNIT_ASSERT(file);
  const std::string data(size, 'x');
  CPPUNIT_ASSERT_EQUAL(size, file.write(data.data(), data.size()));
}

void RotatingLogFileTest::testRotationKeepsEachFileBounded()
{
  RotatingLogFile file(path_, 64, 3);
  file.open();
  CPPUNIT_ASSERT(file.write(std::string(48, 'a')));
  CPPUNIT_ASSERT(file.write(std::string(48, 'b')));
  CPPUNIT_ASSERT(file.write(std::string(200, 'c')));
  file.close();

  CPPUNIT_ASSERT(File(path_).size() <= 64);
  CPPUNIT_ASSERT(File(path_ + ".1").size() <= 64);
  CPPUNIT_ASSERT(File(path_ + ".2").size() <= 64);
}

void RotatingLogFileTest::testRetentionLimitsTotalFiles()
{
  RotatingLogFile file(path_, 8, 2);
  file.open();
  CPPUNIT_ASSERT(file.write("aaaaa"));
  CPPUNIT_ASSERT(file.write("bbbbb"));
  CPPUNIT_ASSERT(file.write("ccccc"));
  CPPUNIT_ASSERT(file.write("ddddd"));
  file.close();

  CPPUNIT_ASSERT(File(path_).exists());
  CPPUNIT_ASSERT(File(path_ + ".1").exists());
  CPPUNIT_ASSERT(!File(path_ + ".2").exists());
  CPPUNIT_ASSERT(File(path_).size() + File(path_ + ".1").size() <= 16);
}

void RotatingLogFileTest::testOpenConvergesLegacyLogs()
{
  writeFile(path_, 100);
  writeFile(path_ + ".1", 100);
  writeFile(path_ + ".2", 5);
  writeFile(path_ + ".3", 5);

  RotatingLogFile file(path_, 10, 2);
  file.open();
  file.close();

  CPPUNIT_ASSERT(File(path_).exists());
  CPPUNIT_ASSERT_EQUAL((int64_t)0, File(path_).size());
  CPPUNIT_ASSERT(!File(path_ + ".1").exists());
  CPPUNIT_ASSERT(!File(path_ + ".2").exists());
  CPPUNIT_ASSERT(!File(path_ + ".3").exists());
}

} // namespace aria2
