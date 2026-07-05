#include <gtest/gtest.h>

#include <string>

#include "async_logger/async_logger.h"

TEST(AsyncLogger, InvalidLogFilePathThrowsException) {
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = "missing_dir/test.log";

  try {
    AsyncLogger::Logger::init(config);
    FAIL() << "Expected FileOpenError to be thrown";
  } catch (const AsyncLogger::FileOpenError& ex) {
    EXPECT_NE(std::string(ex.what()).find(config.filename), std::string::npos);
  }

  AsyncLogger::Logger::shutdown();
}
