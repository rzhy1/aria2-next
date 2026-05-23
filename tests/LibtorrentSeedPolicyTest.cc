#include "LibtorrentSeedPolicy.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Option.h"
#include "a2functional.h"
#include "prefs.h"

namespace aria2 {

class LibtorrentSeedPolicyTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(LibtorrentSeedPolicyTest);
  CPPUNIT_TEST(testStopsWhenSeedTimeElapsed);
  CPPUNIT_TEST(testStopsWhenShareRatioReached);
  CPPUNIT_TEST(testKeepsSeedingWhenRatioDisabled);
  CPPUNIT_TEST_SUITE_END();

public:
  void testStopsWhenSeedTimeElapsed();
  void testStopsWhenShareRatioReached();
  void testKeepsSeedingWhenRatioDisabled();
};

CPPUNIT_TEST_SUITE_REGISTRATION(LibtorrentSeedPolicyTest);

void LibtorrentSeedPolicyTest::testStopsWhenSeedTimeElapsed()
{
  Option option;
  option.put(PREF_SEED_TIME, "0.5");
  option.put(PREF_SEED_RATIO, "10.0");

  CPPUNIT_ASSERT(shouldStopLibtorrentSeeding(
      &option, 100_k, 0, std::chrono::seconds(30)));
  CPPUNIT_ASSERT(!shouldStopLibtorrentSeeding(
      &option, 100_k, 0, std::chrono::seconds(29)));
}

void LibtorrentSeedPolicyTest::testStopsWhenShareRatioReached()
{
  Option option;
  option.put(PREF_SEED_RATIO, "1.5");

  CPPUNIT_ASSERT(shouldStopLibtorrentSeeding(
      &option, 100_k, 150_k, std::chrono::seconds(0)));
  CPPUNIT_ASSERT(!shouldStopLibtorrentSeeding(
      &option, 100_k, 149_k, std::chrono::seconds(0)));
}

void LibtorrentSeedPolicyTest::testKeepsSeedingWhenRatioDisabled()
{
  Option option;
  option.put(PREF_SEED_RATIO, "0.0");

  CPPUNIT_ASSERT(!hasLibtorrentSeedLimit(&option));
  CPPUNIT_ASSERT(!shouldStopLibtorrentSeeding(
      &option, 100_k, 10_m, std::chrono::hours(24)));
}

} // namespace aria2
