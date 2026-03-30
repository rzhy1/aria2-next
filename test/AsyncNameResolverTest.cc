#include "AsyncNameResolver.h"

#include <cstring>

#include <cppunit/extensions/HelperMacros.h>

#include "SocketCore.h"

namespace aria2 {

class AsyncNameResolverTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(AsyncNameResolverTest);
  CPPUNIT_TEST(testUsable);
  CPPUNIT_TEST(testUsableWithExplicitServers);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testUsable();
  void testUsableWithExplicitServers();
};

CPPUNIT_TEST_SUITE_REGISTRATION(AsyncNameResolverTest);

void AsyncNameResolverTest::testUsable()
{
  // On a normal development machine, c-ares should be able to detect
  // system DNS servers and usable() should return true.
  // This test will fail on Windows ARM64 systems where c-ares cannot
  // detect DNS servers — that is the exact scenario our fallback
  // mechanism is designed to handle.
  AsyncNameResolver resolver(AF_INET, "");
  CPPUNIT_ASSERT(resolver.usable());
}

void AsyncNameResolverTest::testUsableWithExplicitServers()
{
  // When explicit DNS servers are provided, the usable check is
  // skipped because we trust the user-supplied configuration.
  // usable() should always return true in this case regardless of
  // whether the system can provide DNS server configuration.
  AsyncNameResolver resolver(AF_INET, "8.8.8.8,8.8.4.4");
  CPPUNIT_ASSERT(resolver.usable());
}

} // namespace aria2
