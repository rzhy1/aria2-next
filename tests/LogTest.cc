#include "Log.h"

#include <cppunit/extensions/HelperMacros.h>

#include "File.h"
#include "Option.h"
#include "prefs.h"
#include "TestUtil.h"

namespace aria2 {

class LogTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(LogTest);
  CPPUNIT_TEST(testOptionLogLevelConfiguresUnifiedLevel);
  CPPUNIT_TEST(testLogLevelFiltersEverySink);
  CPPUNIT_TEST_SUITE_END();

public:
  void testOptionLogLevelConfiguresUnifiedLevel();
  void testLogLevelFiltersEverySink();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LogTest);

void LogTest::testOptionLogLevelConfiguresUnifiedLevel()
{
  Option option;
  option.put(PREF_LOG_FILE, V_OFF);
  option.put(PREF_LOG_LEVEL, V_WARN);
  option.put(PREF_LOG_MAX_SIZE, "1048576");
  option.put(PREF_LOG_MAX_FILES, "2");
  option.put(PREF_ENABLE_COLOR, A2_V_FALSE);
  option.put(PREF_QUIET, A2_V_FALSE);

  auto config = log::configFromOption(option);

  CPPUNIT_ASSERT(config.console);
  CPPUNIT_ASSERT_EQUAL(log::Level::Warn, config.level);
}

void LogTest::testLogLevelFiltersEverySink()
{
  std::string logfile = A2_TEST_OUT_DIR "/aria2_LogTest.log";
  File(logfile).remove();

  log::Config config;
  config.file = logfile;
  config.level = log::Level::Warn;
  config.console = false;
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
