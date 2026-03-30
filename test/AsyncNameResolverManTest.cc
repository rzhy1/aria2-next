#include "AsyncNameResolverMan.h"

#include <cppunit/extensions/HelperMacros.h>

#include "Option.h"
#include "prefs.h"
#include "SocketCore.h"

namespace aria2 {

class AsyncNameResolverManTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AsyncNameResolverManTest);
  CPPUNIT_TEST(testConfigureDefaults);
  CPPUNIT_TEST(testConfigureWithExplicitServers);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testConfigureDefaults();
  void testConfigureWithExplicitServers();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AsyncNameResolverManTest);

void AsyncNameResolverManTest::testConfigureDefaults()
{
  // Verify that AsyncNameResolverMan can be constructed and configured
  // with default options without crashing.
  AsyncNameResolverMan man;
  Option option;

  // On a system with working DNS configuration, the configuration
  // should succeed without issues.
  configureAsyncNameResolverMan(&man, &option);

  // No resolvers should be started yet
  CPPUNIT_ASSERT(!man.started());
}

void AsyncNameResolverManTest::testConfigureWithExplicitServers()
{
  // When explicit DNS servers are configured, the resolver should
  // always be considered usable regardless of the system's DNS
  // configuration.
  AsyncNameResolverMan man;
  Option option;
  option.put(PREF_ASYNC_DNS_SERVER, "8.8.8.8,8.8.4.4");

  configureAsyncNameResolverMan(&man, &option);

  CPPUNIT_ASSERT(!man.started());
}

} // namespace aria2
