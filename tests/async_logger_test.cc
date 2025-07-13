#include "async_logger/async_logger.h"

#include <gtest/gtest.h>

#include <cstdio>
#include <string>

namespace {
static const std::string tmp_log = "test.log";

std::string readFile(const std::string& filename) {
  std::ifstream ifs(filename);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  return content;
}
}  // namespace

TEST(AsyncLogger, LevelFiltering) {
  // Init with level WARN, debug/info should be filtered out

  AsyncLogger::Logger::init(tmp_log, AsyncLogger::Level::Warn);
  AsyncLogger::Logger::debug("debug message");
  AsyncLogger::Logger::info("info message");
  AsyncLogger::Logger::warn("warn message");
  AsyncLogger::Logger::error("error message");
  AsyncLogger::Logger::shutdown();

  std::string content = readFile(tmp_log);
  EXPECT_EQ(content.find("debug message"), std::string::npos);
  EXPECT_EQ(content.find("info message"), std::string::npos);
  EXPECT_NE(content.find("warn message"), std::string::npos);
  EXPECT_NE(content.find("error message"), std::string::npos);
  std::remove(tmp_log.c_str());
}

TEST(AsyncLogger, ThreadSafety) {
  AsyncLogger::Logger::init(tmp_log, AsyncLogger::Level::Info);
  // Launch multiple threads to log concurrently
  std::vector<std::thread> threads;
  for (int i = 0; i < 5; ++i) {
    threads.emplace_back(
        [i]() { AsyncLogger::Logger::info("thread " + std::to_string(i)); });
  }
  for (auto& th : threads) th.join();
  AsyncLogger::Logger::shutdown();

  std::string content = readFile(tmp_log);
  for (int i = 0; i < 5; ++i) {
    EXPECT_NE(content.find("thread " + std::to_string(i)), std::string::npos);
  }
  std::remove(tmp_log.c_str());
}
