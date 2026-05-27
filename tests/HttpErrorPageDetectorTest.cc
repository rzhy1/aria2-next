#include "HttpErrorPageDetector.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class HttpErrorPageDetectorTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(HttpErrorPageDetectorTest);
  CPPUNIT_TEST(testRejectsHtmlForBinaryTarget);
  CPPUNIT_TEST(testRejectsGoogleDriveConfirmForBinaryTarget);
  CPPUNIT_TEST(testRejectsCloudflareChallengeForBinaryTarget);
  CPPUNIT_TEST(testAllowsHtmlTarget);
  CPPUNIT_TEST(testAllowsBinaryContentTypeForBinaryTarget);
  CPPUNIT_TEST_SUITE_END();

public:
  void testRejectsHtmlForBinaryTarget();
  void testRejectsGoogleDriveConfirmForBinaryTarget();
  void testRejectsCloudflareChallengeForBinaryTarget();
  void testAllowsHtmlTarget();
  void testAllowsBinaryContentTypeForBinaryTarget();
};

CPPUNIT_TEST_SUITE_REGISTRATION(HttpErrorPageDetectorTest);

void HttpErrorPageDetectorTest::testRejectsHtmlForBinaryTarget()
{
  auto decision = detectHttpErrorPage(
      "installer.exe", "text/html; charset=utf-8",
      "<!doctype html><html><title>Sign in</title><form>login</form>");

  CPPUNIT_ASSERT(decision.reject);
  CPPUNIT_ASSERT(!decision.reason.empty());
}

void HttpErrorPageDetectorTest::testRejectsGoogleDriveConfirmForBinaryTarget()
{
  auto decision = detectHttpErrorPage(
      "archive.zip", "", "<html><a id=\"uc-download-link\" "
                         "href=\"/uc?confirm=t\">download</a></html>");

  CPPUNIT_ASSERT(decision.reject);
}

void HttpErrorPageDetectorTest::testRejectsCloudflareChallengeForBinaryTarget()
{
  auto decision = detectHttpErrorPage(
      "image.iso", "text/html",
      "<html><title>Just a moment...</title><script>cf-chl-bypass</script>");

  CPPUNIT_ASSERT(decision.reject);
}

void HttpErrorPageDetectorTest::testAllowsHtmlTarget()
{
  auto decision = detectHttpErrorPage(
      "index.html", "text/html", "<!doctype html><html>Hello</html>");

  CPPUNIT_ASSERT(!decision.reject);
}

void HttpErrorPageDetectorTest::testAllowsBinaryContentTypeForBinaryTarget()
{
  auto decision = detectHttpErrorPage(
      "restore.ipsw", "application/octet-stream", "PK\003\004payload");

  CPPUNIT_ASSERT(!decision.reject);
}

} // namespace aria2
