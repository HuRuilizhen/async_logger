#include <gtest/gtest.h>

#include <cstdio>
#include <string>

#include "async_logger/async_logger.h"
#include "async_logger_test_utils.h"

TEST(AsyncLogger, LevelFiltering) {
  // Init with level WARN, debug/info should be filtered out
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Warn;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = AsyncLoggerTest::kTempLog;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::debug("debug message");
  AsyncLogger::Logger::info("info message");
  AsyncLogger::Logger::warn("warn message");
  AsyncLogger::Logger::error("error message");
  AsyncLogger::Logger::shutdown();

  std::string content = AsyncLoggerTest::readFile(AsyncLoggerTest::kTempLog);
  EXPECT_EQ(content.find("debug message"), std::string::npos);
  EXPECT_EQ(content.find("info message"), std::string::npos);
  EXPECT_NE(content.find("warn message"), std::string::npos);
  EXPECT_NE(content.find("error message"), std::string::npos);
  std::remove(AsyncLoggerTest::kTempLog.c_str());
}

TEST(AsyncLogger, ThreadSafety) {
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = AsyncLoggerTest::kTempLog;

  AsyncLogger::Logger::init(config);

  // Launch multiple threads to log concurrently
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; ++i) {
    threads.emplace_back(
        [i]() { AsyncLogger::Logger::info("thread " + std::to_string(i)); });
  }
  for (auto& th : threads) th.join();
  AsyncLogger::Logger::shutdown();

  std::string content = AsyncLoggerTest::readFile(AsyncLoggerTest::kTempLog);
  for (int i = 0; i < 5; ++i) {
    EXPECT_NE(content.find("thread " + std::to_string(i)), std::string::npos);
  }
  std::remove(AsyncLoggerTest::kTempLog.c_str());
}
