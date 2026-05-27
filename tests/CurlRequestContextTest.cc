#include "CurlRequestContext.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class CurlRequestContextTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(CurlRequestContextTest);
  CPPUNIT_TEST(testTaskScopedScalarHeadersWin);
  CPPUNIT_TEST(testSemanticHeadersAreNotDuplicated);
  CPPUNIT_TEST(testHeaderOrderIsStable);
  CPPUNIT_TEST(testExplicitAcceptEncodingUsesCurlDecoderPath);
  CPPUNIT_TEST(testDefaultAcceptEncodingPolicy);
  CPPUNIT_TEST_SUITE_END();

public:
  void testTaskScopedScalarHeadersWin();
  void testSemanticHeadersAreNotDuplicated();
  void testHeaderOrderIsStable();
  void testExplicitAcceptEncodingUsesCurlDecoderPath();
  void testDefaultAcceptEncodingPolicy();
};

CPPUNIT_TEST_SUITE_REGISTRATION(CurlRequestContextTest);

void CurlRequestContextTest::testTaskScopedScalarHeadersWin()
{
  CurlRequestContextInput input;
  input.userAgent = "TaskAgent/1.0";
  input.referer = "https://example.test/task";
  input.httpAcceptGzip = true;

  auto context = buildCurlRequestContext(input);

  CPPUNIT_ASSERT_EQUAL(std::string("TaskAgent/1.0"), context.userAgent);
  CPPUNIT_ASSERT(context.hasReferer);
  CPPUNIT_ASSERT_EQUAL(std::string("https://example.test/task"),
                       context.referer);
}

void CurlRequestContextTest::testSemanticHeadersAreNotDuplicated()
{
  CurlRequestContextInput input;
  input.userAgent = "GlobalAgent/1.0";
  input.referer = "https://example.test/global";
  input.httpAcceptGzip = true;
  input.headers = {"User-Agent: TaskAgent/2.0",
                   "Referer: https://example.test/task",
                   "Cookie: sid=abc; token=def",
                   "X-Trace: one"};

  auto context = buildCurlRequestContext(input);

  CPPUNIT_ASSERT_EQUAL(std::string("TaskAgent/2.0"), context.userAgent);
  CPPUNIT_ASSERT_EQUAL(std::string("https://example.test/task"),
                       context.referer);
  CPPUNIT_ASSERT(context.hasCookie);
  CPPUNIT_ASSERT_EQUAL(std::string("sid=abc; token=def"), context.cookie);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), context.headers.size());
  CPPUNIT_ASSERT_EQUAL(std::string("X-Trace: one"), context.headers[0]);
}

void CurlRequestContextTest::testHeaderOrderIsStable()
{
  CurlRequestContextInput input;
  input.httpAcceptGzip = false;
  input.headers = {"X-First: 1", "Authorization: Bearer token", "X-Last: 3"};

  auto context = buildCurlRequestContext(input);

  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), context.headers.size());
  CPPUNIT_ASSERT_EQUAL(std::string("X-First: 1"), context.headers[0]);
  CPPUNIT_ASSERT_EQUAL(std::string("Authorization: Bearer token"),
                       context.headers[1]);
  CPPUNIT_ASSERT_EQUAL(std::string("X-Last: 3"), context.headers[2]);
}

void CurlRequestContextTest::testExplicitAcceptEncodingUsesCurlDecoderPath()
{
  CurlRequestContextInput input;
  input.httpAcceptGzip = false;
  input.headers = {"Accept-Encoding: br, gzip", "X-Download: yes"};

  auto context = buildCurlRequestContext(input);

  CPPUNIT_ASSERT(context.hasAcceptEncoding);
  CPPUNIT_ASSERT_EQUAL(std::string("br, gzip"), context.acceptEncoding);
  CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(1), context.headers.size());
  CPPUNIT_ASSERT_EQUAL(std::string("X-Download: yes"), context.headers[0]);
}

void CurlRequestContextTest::testDefaultAcceptEncodingPolicy()
{
  CurlRequestContextInput gzipInput;
  gzipInput.httpAcceptGzip = true;
  auto gzipContext = buildCurlRequestContext(gzipInput);
  CPPUNIT_ASSERT(gzipContext.hasAcceptEncoding);
  CPPUNIT_ASSERT_EQUAL(std::string(""), gzipContext.acceptEncoding);

  CurlRequestContextInput identityInput;
  identityInput.httpAcceptGzip = false;
  auto identityContext = buildCurlRequestContext(identityInput);
  CPPUNIT_ASSERT(identityContext.hasAcceptEncoding);
  CPPUNIT_ASSERT_EQUAL(std::string("identity"), identityContext.acceptEncoding);
}

} // namespace aria2
