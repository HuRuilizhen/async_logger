#include <gtest/gtest.h>

#include <atomic>
#include <ctime>
#include <string>
#include <thread>

#include "async_logger/async_logger.h"
#include "async_logger_test_utils.h"

namespace {
std::atomic<int> g_timestamp_call_count{0};

std::tm makeFixedTime(int sec) {
  std::tm time{};
  time.tm_year = 124;
  time.tm_mon = 0;
  time.tm_mday = 2;
  time.tm_hour = 3;
  time.tm_min = 4;
  time.tm_sec = sec;
  return time;
}

const std::tm sequencedTimeFunc() {
  int call = g_timestamp_call_count.fetch_add(1, std::memory_order_relaxed);
  return call == 0 ? makeFixedTime(5) : makeFixedTime(6);
}

const std::tm newTimeFunc() {
  std::tm time = AsyncLogger::LoggerUtils::getCurrentTime();
  time.tm_mday += 1;
  return time;
}

}  // namespace

TEST(AsyncLogger, EmptyFileName) {
  AsyncLogger::Logger::info("test");
  AsyncLogger::Logger::shutdown();

  std::tm tm = AsyncLogger::LoggerUtils::getCurrentTime();
  char filename[20];
  std::strftime(filename, sizeof(filename), "%Y-%m-%d.log", &tm);

  std::string content = AsyncLoggerTest::readFile(filename);
  EXPECT_NE(content.find("test"), std::string::npos);

  std::remove(filename);
}

TEST(AsyncLogger, AppendFileMode) {
  // Init with mode append
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file |
                AsyncLogger::OutstreamFlag::mode_append;
  config.filename = AsyncLoggerTest::kTempLog;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("alpha");
  AsyncLogger::Logger::shutdown();

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("beta");
  AsyncLogger::Logger::shutdown();

  std::string content = AsyncLoggerTest::readFile(AsyncLoggerTest::kTempLog);
  EXPECT_NE(content.find("alpha"), std::string::npos);
  EXPECT_NE(content.find("beta"), std::string::npos);

  std::remove(AsyncLoggerTest::kTempLog.c_str());
}

TEST(AsyncLogger, TruncFileMode) {
  // Init with mode trunc
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = AsyncLoggerTest::kTempLog;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("alpha");
  AsyncLogger::Logger::shutdown();

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("beta");
  AsyncLogger::Logger::shutdown();

  std::string content = AsyncLoggerTest::readFile(AsyncLoggerTest::kTempLog);
  EXPECT_EQ(content.find("alpha"), std::string::npos);
  EXPECT_NE(content.find("beta"), std::string::npos);

  std::remove(AsyncLoggerTest::kTempLog.c_str());
}

TEST(AsyncLogger, LogFileRotation) {
  AsyncLoggerTest::TimeFuncGuard time_guard;
  AsyncLogger::Logger::info("alpha");
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::string filename = AsyncLogger::LoggerUtils::getDefaultFilename();
  std::string content = AsyncLoggerTest::readFile(filename);
  EXPECT_NE(content.find("alpha"), std::string::npos);
  std::remove(filename.c_str());

  AsyncLogger::LoggerUtils::timeFuncPtr = &newTimeFunc;
  AsyncLogger::Logger::info("beta");
  AsyncLogger::Logger::shutdown();

  filename = AsyncLogger::LoggerUtils::getDefaultFilename();
  content = AsyncLoggerTest::readFile(filename);
  EXPECT_NE(content.find("beta"), std::string::npos);
  std::remove(filename.c_str());
}

TEST(AsyncLogger, FileOutputUsesEnqueueTimestamp) {
  AsyncLoggerTest::TimeFuncGuard time_guard;
  g_timestamp_call_count.store(0, std::memory_order_relaxed);
  AsyncLogger::LoggerUtils::timeFuncPtr = &sequencedTimeFunc;

  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = AsyncLoggerTest::kTempLog;
  config.wait_strategy = AsyncLogger::WaitStrategy::Blocking;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("captured timestamp");
  AsyncLogger::Logger::shutdown();

  std::string content = AsyncLoggerTest::readFile(AsyncLoggerTest::kTempLog);
  EXPECT_NE(content.find(AsyncLoggerTest::formatTime(makeFixedTime(5))),
            std::string::npos);
  EXPECT_EQ(content.find(AsyncLoggerTest::formatTime(makeFixedTime(6))),
            std::string::npos);
  std::remove(AsyncLoggerTest::kTempLog.c_str());
}

TEST(AsyncLogger, FileOutputNeverContainsAnsiColorCodes) {
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file |
                AsyncLogger::OutstreamFlag::out_color;
  config.filename = AsyncLoggerTest::kTempLog;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::error("plain file output");
  AsyncLogger::Logger::shutdown();

  std::string content = AsyncLoggerTest::readFile(AsyncLoggerTest::kTempLog);
  EXPECT_NE(content.find("plain file output"), std::string::npos);
  EXPECT_EQ(content.find("\033["), std::string::npos);
  std::remove(AsyncLoggerTest::kTempLog.c_str());
}
