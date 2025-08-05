#include <gtest/gtest.h>

#include <cstdio>
#include <string>

#include "async_logger/async_logger.h"

namespace {
static const std::string TMPLOG = "test.log";

std::string readFile(const std::string& filename) {
  std::ifstream ifs(filename);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  return content;
}
}  // namespace

TEST(AsyncLogger, LevelFiltering) {
  // Init with level WARN, debug/info should be filtered out

  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Warn;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = TMPLOG;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::debug("debug message");
  AsyncLogger::Logger::info("info message");
  AsyncLogger::Logger::warn("warn message");
  AsyncLogger::Logger::error("error message");
  AsyncLogger::Logger::shutdown();

  std::string content = readFile(TMPLOG);
  EXPECT_EQ(content.find("debug message"), std::string::npos);
  EXPECT_EQ(content.find("info message"), std::string::npos);
  EXPECT_NE(content.find("warn message"), std::string::npos);
  EXPECT_NE(content.find("error message"), std::string::npos);
  std::remove(TMPLOG.c_str());
}

TEST(AsyncLogger, ThreadSafety) {
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = TMPLOG;

  AsyncLogger::Logger::init(config);

  // Launch multiple threads to log concurrently
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; ++i) {
    threads.emplace_back(
        [i]() { AsyncLogger::Logger::info("thread " + std::to_string(i)); });
  }
  for (auto& th : threads) th.join();
  AsyncLogger::Logger::shutdown();

  std::string content = readFile(TMPLOG);
  for (int i = 0; i < 5; ++i) {
    EXPECT_NE(content.find("thread " + std::to_string(i)), std::string::npos);
  }
  std::remove(TMPLOG.c_str());
}
