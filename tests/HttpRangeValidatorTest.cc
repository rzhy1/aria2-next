#include "HttpRangeValidator.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class HttpRangeValidatorTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(HttpRangeValidatorTest);
  CPPUNIT_TEST(testAcceptsMatchingPartialContent);
  CPPUNIT_TEST(testDetectsIgnoredRangeResponse);
  CPPUNIT_TEST(testRejectsMismatchedContentRange);
  CPPUNIT_TEST(testRejectsEncodedRangeResponse);
  CPPUNIT_TEST(testRejectsEncodedHeadMetadataLength);
  CPPUNIT_TEST(testAcceptsRangeMetadataProbeLength);
  CPPUNIT_TEST_SUITE_END();

public:
  void testAcceptsMatchingPartialContent();
  void testDetectsIgnoredRangeResponse();
  void testRejectsMismatchedContentRange();
  void testRejectsEncodedRangeResponse();
  void testRejectsEncodedHeadMetadataLength();
  void testAcceptsRangeMetadataProbeLength();
};

CPPUNIT_TEST_SUITE_REGISTRATION(HttpRangeValidatorTest);

void HttpRangeValidatorTest::testAcceptsMatchingPartialContent()
{
  auto result = validateHttpRangeResponse(
      206, Range(1024, 2047, 4096), Range(1024, 2047, 4096), 4096, "");

  CPPUNIT_ASSERT(result.ok);
}

void HttpRangeValidatorTest::testDetectsIgnoredRangeResponse()
{
  auto result = validateHttpRangeResponse(
      200, Range(0, 1023, 4096), Range(0, 4095, 4096), 4096, "");

  CPPUNIT_ASSERT(!result.ok);
  CPPUNIT_ASSERT(result.rangeUnsupported);
}

void HttpRangeValidatorTest::testRejectsMismatchedContentRange()
{
  auto result = validateHttpRangeResponse(
      206, Range(1024, 2047, 4096), Range(0, 1023, 4096), 4096, "");

  CPPUNIT_ASSERT(!result.ok);
  CPPUNIT_ASSERT(result.retryable);
}

void HttpRangeValidatorTest::testRejectsEncodedRangeResponse()
{
  auto result = validateHttpRangeResponse(
      206, Range(0, 1023, 4096), Range(0, 1023, 4096), 4096, "gzip");

  CPPUNIT_ASSERT(!result.ok);
}

void HttpRangeValidatorTest::testRejectsEncodedHeadMetadataLength()
{
  auto result = validateHttpMetadataHead(200, 20, "gzip");

  CPPUNIT_ASSERT(!result.ok);
  CPPUNIT_ASSERT(result.needsRangeProbe);
}

void HttpRangeValidatorTest::testAcceptsRangeMetadataProbeLength()
{
  auto result =
      validateHttpMetadataRangeProbe(206, Range(0, 0, 987896072), "identity");

  CPPUNIT_ASSERT(result.ok);
  CPPUNIT_ASSERT_EQUAL(static_cast<int64_t>(987896072), result.entityLength);
}

} // namespace aria2
