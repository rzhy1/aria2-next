#include "Log.h"

#include <cppunit/extensions/HelperMacros.h>

#include "BufferedFile.h"
#include "File.h"

namespace aria2 {

class LogTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(LogTest);
  CPPUNIT_TEST(testRotationKeepsStrictBounds);
  CPPUNIT_TEST(testStartupConvergesLegacyLogs);
  CPPUNIT_TEST(testOversizedRecordIsBounded);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testRotationKeepsStrictBounds();
  void testStartupConvergesLegacyLogs();
  void testOversizedRecordIsBounded();

private:
  std::string path_;
  logging::Settings originalSettings_;

  logging::Settings settings(size_t maxSize, size_t maxFiles) const;
  void removeLogs();
  void writeFile(const std::string& path, size_t size);
};

CPPUNIT_TEST_SUITE_REGISTRATION(LogTest);

void LogTest::setUp()
{
  path_ = A2_TEST_OUT_DIR "/aria2_LogTest.log";
  originalSettings_ = logging::getSettings();
  logging::shutdown();
  removeLogs();
}

void LogTest::tearDown()
{
  logging::shutdown();
  removeLogs();
  logging::configure(originalSettings_);
}

logging::Settings LogTest::settings(size_t maxSize, size_t maxFiles) const
{
  logging::Settings settings;
  settings.file = path_;
  settings.maxFileSize = maxSize;
  settings.maxFiles = maxFiles;
  settings.fileLevel = spdlog::level::trace;
  settings.consoleOutput = false;
  return settings;
}

void LogTest::removeLogs()
{
  File(path_).remove();
  for (size_t i = 1; i <= logging::MAX_FILES; ++i) {
    File(path_ + "." + std::to_string(i)).remove();
    File(A2_TEST_OUT_DIR "/aria2_LogTest." + std::to_string(i) + ".log")
        .remove();
  }
}

void LogTest::writeFile(const std::string& path, size_t size)
{
  BufferedFile file(path.c_str(), BufferedFile::WRITE);
  CPPUNIT_ASSERT(file);
  const std::string data(size, 'x');
  CPPUNIT_ASSERT_EQUAL(size, file.write(data.data(), data.size()));
}

void LogTest::testRotationKeepsStrictBounds()
{
  logging::configure(settings(128, 2));
  for (size_t i = 0; i < 12; ++i) {
    A2_LOG_TRACE(std::string(48, static_cast<char>('a' + i)));
  }
  logging::flush();

  const std::string history = A2_TEST_OUT_DIR "/aria2_LogTest.1.log";
  CPPUNIT_ASSERT(File(path_).exists());
  CPPUNIT_ASSERT(File(history).exists());
  CPPUNIT_ASSERT(!File(A2_TEST_OUT_DIR "/aria2_LogTest.2.log").exists());
  CPPUNIT_ASSERT(File(path_).size() <= 128);
  CPPUNIT_ASSERT(File(history).size() <= 128);
  CPPUNIT_ASSERT(File(path_).size() + File(history).size() <= 256);
}

void LogTest::testStartupConvergesLegacyLogs()
{
  const std::string nativeHistory =
      A2_TEST_OUT_DIR "/aria2_LogTest.1.log";
  writeFile(path_, 256);
  writeFile(path_ + ".1", 256);
  writeFile(nativeHistory, 256);
  writeFile(A2_TEST_OUT_DIR "/aria2_LogTest.2.log", 8);

  logging::configure(settings(64, 2));
  logging::flush();

  CPPUNIT_ASSERT(File(path_).exists());
  CPPUNIT_ASSERT_EQUAL((int64_t)0, File(path_).size());
  CPPUNIT_ASSERT(!File(path_ + ".1").exists());
  CPPUNIT_ASSERT(!File(nativeHistory).exists());
  CPPUNIT_ASSERT(!File(A2_TEST_OUT_DIR "/aria2_LogTest.2.log").exists());
}

void LogTest::testOversizedRecordIsBounded()
{
  logging::configure(settings(96, 1));
  A2_LOG_ERROR(std::string(4096, 'x'));
  logging::flush();

  CPPUNIT_ASSERT(File(path_).exists());
  CPPUNIT_ASSERT(File(path_).size() <= 96);
  CPPUNIT_ASSERT(!File(A2_TEST_OUT_DIR "/aria2_LogTest.1.log").exists());
}

} // namespace aria2
