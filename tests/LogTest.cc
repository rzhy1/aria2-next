#include "Log.h"

#include <cppunit/extensions/HelperMacros.h>

#include "File.h"
#include "TestUtil.h"

namespace aria2 {

class LogTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(LogTest);
  CPPUNIT_TEST(testRotatingFileLogWritesConfiguredLevel);
  CPPUNIT_TEST_SUITE_END();

public:
  void testRotatingFileLogWritesConfiguredLevel();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LogTest);

void LogTest::testRotatingFileLogWritesConfiguredLevel()
{
  std::string logfile = A2_TEST_OUT_DIR "/aria2_LogTest.log";
  File(logfile).remove();

  log::Config config;
  config.file = logfile;
  config.fileLevel = log::Level::Warn;
  config.consoleLevel = log::Level::Off;
  config.maxFileSize = 1024 * 1024;
  config.maxFiles = 2;
  log::configure(config);

  ARIA2_LOG_INFO("filtered-info-record");
  ARIA2_LOG_WARN("written-warn-record");
  log::shutdown();

  std::string contents = readFile(logfile);
  CPPUNIT_ASSERT(contents.find("written-warn-record") != std::string::npos);
  CPPUNIT_ASSERT(contents.find("filtered-info-record") == std::string::npos);
}

} // namespace aria2
