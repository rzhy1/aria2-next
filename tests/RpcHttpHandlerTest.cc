#include "RpcHttpHandler.h"

#include <cppunit/extensions/HelperMacros.h>

#include "DownloadEngine.h"
#include "Option.h"
#include "RequestGroupMan.h"
#include "SelectEventPoll.h"
#include "prefs.h"

namespace aria2 {

class RpcHttpHandlerTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RpcHttpHandlerTest);
  CPPUNIT_TEST(testCorsPreflight);
  CPPUNIT_TEST(testCorsHeaderOnJsonRpcPostResponse);
  CPPUNIT_TEST(testRpcSecretRejectsMissingToken);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    option_ = make_unique<Option>();
    engine_ = make_unique<DownloadEngine>(make_unique<SelectEventPoll>());
    engine_->setOption(option_.get());
    engine_->setRequestGroupMan(make_unique<RequestGroupMan>(
        std::vector<std::shared_ptr<RequestGroup>>{}, 1, option_.get()));
  }

  void testCorsPreflight();
  void testCorsHeaderOnJsonRpcPostResponse();
  void testRpcSecretRejectsMissingToken();

private:
  std::unique_ptr<Option> option_;
  std::unique_ptr<DownloadEngine> engine_;
};

CPPUNIT_TEST_SUITE_REGISTRATION(RpcHttpHandlerTest);

void RpcHttpHandlerTest::testCorsPreflight()
{
  option_->put(PREF_RPC_ALLOW_ORIGIN_ALL, A2_V_TRUE);
  RpcHttpHandler handler(engine_.get());

  RpcHttpRequest req;
  req.method = "OPTIONS";
  req.target = "/jsonrpc";
  req.headers.emplace("origin", "http://example.test");
  req.headers.emplace("access-control-request-method", "POST");
  req.headers.emplace("access-control-request-headers", "content-type");

  auto res = handler.handle(req);

  CPPUNIT_ASSERT_EQUAL(200, res.status);
  CPPUNIT_ASSERT_EQUAL(std::string("*"),
                       res.headers["access-control-allow-origin"]);
  CPPUNIT_ASSERT_EQUAL(std::string("POST, GET, OPTIONS"),
                       res.headers["access-control-allow-methods"]);
  CPPUNIT_ASSERT_EQUAL(std::string("content-type"),
                       res.headers["access-control-allow-headers"]);
}

void RpcHttpHandlerTest::testCorsHeaderOnJsonRpcPostResponse()
{
  option_->put(PREF_RPC_ALLOW_ORIGIN_ALL, A2_V_TRUE);
  option_->put(PREF_RPC_SECRET, "secret");
  RpcHttpHandler handler(engine_.get());

  RpcHttpRequest req;
  req.method = "POST";
  req.target = "/jsonrpc";
  req.body = "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"method\":\"aria2.getVersion\"}";

  auto res = handler.handle(req);

  CPPUNIT_ASSERT_EQUAL(400, res.status);
  CPPUNIT_ASSERT_EQUAL(std::string("*"),
                       res.headers["access-control-allow-origin"]);
  CPPUNIT_ASSERT(res.body.find("Unauthorized") != std::string::npos);
}

void RpcHttpHandlerTest::testRpcSecretRejectsMissingToken()
{
  option_->put(PREF_RPC_SECRET, "secret");
  RpcHttpHandler handler(engine_.get());

  RpcHttpRequest req;
  req.method = "POST";
  req.target = "/jsonrpc";
  req.body = "{\"jsonrpc\":\"2.0\",\"id\":\"1\",\"method\":\"aria2.getVersion\"}";

  auto res = handler.handle(req);

  CPPUNIT_ASSERT_EQUAL(400, res.status);
  CPPUNIT_ASSERT(res.closeConnection);
  CPPUNIT_ASSERT(res.body.find("Unauthorized") != std::string::npos);
}

} // namespace aria2
