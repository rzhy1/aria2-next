#include "common.h"

#include <cppunit/extensions/HelperMacros.h>

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "console.h"
#include "OptionHandlerFactory.h"
#include "OptionParser.h"
#include "OutputFile.h"

namespace aria2 {

void showVersion();
void showUsage(const std::string& keyword,
               const std::shared_ptr<OptionParser>& oparser,
               const Console& out);

class StringOutputFile : public OutputFile {
public:
  std::string out;

  size_t write(const char* str) CXX11_OVERRIDE
  {
    out += str;
    return strlen(str);
  }

  int flush() CXX11_OVERRIDE { return 0; }

  int vprintf(const char* format, va_list va) CXX11_OVERRIDE
  {
    va_list copy;
    va_copy(copy, va);
    const int len = vsnprintf(nullptr, 0, format, copy);
    va_end(copy);
    if (len < 0) {
      return len;
    }
    std::vector<char> buf(static_cast<size_t>(len) + 1);
    vsnprintf(buf.data(), buf.size(), format, va);
    out.append(buf.data(), static_cast<size_t>(len));
    return len;
  }

  bool supportsColor() CXX11_OVERRIDE { return false; }
};

class VersionUsageTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(VersionUsageTest);
  CPPUNIT_TEST(testShowVersionDisplaysMaintainedForkIdentity);
  CPPUNIT_TEST(testShowUsageUsesProductNameForProse);
  CPPUNIT_TEST_SUITE_END();

public:
  void testShowVersionDisplaysMaintainedForkIdentity();
  void testShowUsageUsesProductNameForProse();
};

CPPUNIT_TEST_SUITE_REGISTRATION(VersionUsageTest);

void VersionUsageTest::testShowVersionDisplaysMaintainedForkIdentity()
{
  std::ostringstream out;
  auto* old = std::cout.rdbuf(out.rdbuf());
  showVersion();
  std::cout.rdbuf(old);

  const auto version = out.str();
  CPPUNIT_ASSERT(version.find("Aria2 Next version " PACKAGE_VERSION) !=
                 std::string::npos);
  CPPUNIT_ASSERT(version.find("Maintained since 2026 by AnInsomniacy") !=
                 std::string::npos);
  CPPUNIT_ASSERT(version.find(
                     "Original aria2 copyright: 2006, 2019 Tatsuhiro "
                     "Tsujikawa.") != std::string::npos);
  CPPUNIT_ASSERT(version.find("Report bugs to "
                              "https://github.com/AnInsomniacy/aria2-next/"
                              "issues") != std::string::npos);
}

void VersionUsageTest::testShowUsageUsesProductNameForProse()
{
  auto parser = std::make_shared<OptionParser>();
  parser->setOptionHandlers(OptionHandlerFactory::createOptionHandlers());
  auto output = std::make_shared<StringOutputFile>();

  showUsage("#all", parser, output);

  CPPUNIT_ASSERT(output->out.find("Aria2 Next saves a control file") !=
                 std::string::npos);
  CPPUNIT_ASSERT(output->out.find("aria2 saves a control file") ==
                 std::string::npos);
  CPPUNIT_ASSERT(output->out.find("Make Aria2 Next quiet") !=
                 std::string::npos);
  CPPUNIT_ASSERT(output->out.find("Make aria2 quiet") == std::string::npos);
  CPPUNIT_ASSERT(output->out.find("aria2.addTorrent") != std::string::npos);
  CPPUNIT_ASSERT(output->out.find("*.aria2") != std::string::npos);
  CPPUNIT_ASSERT(output->out.find("aria2.conf") != std::string::npos);
}

} // namespace aria2
