#include "async_logger/async_logger_macro.h"

#include <gtest/gtest.h>

#include "async_logger/async_logger.h"
#include "async_logger_test_utils.h"

TEST(AsyncLogger, FormattedMacroSupportsLiteralOnly) {
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = AsyncLoggerTest::kTempLog;

  AsyncLogger::Logger::init(config);
  LOGF_INFO("literal only");
  AsyncLogger::Logger::shutdown();

  std::string content = AsyncLoggerTest::readFile(AsyncLoggerTest::kTempLog);
  EXPECT_NE(content.find("literal only"), std::string::npos);
  std::remove(AsyncLoggerTest::kTempLog.c_str());
}
