#include <gtest/gtest.h>

#include <cstdio>
#include <string>

#include "async_logger/async_logger.h"
#include "async_logger_test_utils.h"

namespace {

void emitBurstLogs(int thread_count, int messages_per_thread) {
  std::vector<std::thread> threads;
  threads.reserve(thread_count);
  for (int i = 0; i < thread_count; ++i) {
    threads.emplace_back([messages_per_thread]() {
      for (int j = 0; j < messages_per_thread; ++j) {
        AsyncLogger::Logger::info("dropped message");
      }
    });
  }
  for (auto& th : threads) th.join();
}

}  // namespace

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

TEST(AsyncLogger, YieldingWaitStrategyStillLogsMessages) {
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = AsyncLoggerTest::kTempLog;
  config.wait_strategy = AsyncLogger::WaitStrategy::Yielding;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("yielding strategy");
  AsyncLogger::Logger::shutdown();

  std::string content = AsyncLoggerTest::readFile(AsyncLoggerTest::kTempLog);
  EXPECT_NE(content.find("yielding strategy"), std::string::npos);
  std::remove(AsyncLoggerTest::kTempLog.c_str());
}

TEST(AsyncLogger, DroppedCountStartsAtZeroAndCanBeReset) {
  AsyncLogger::LoggerTestPeer::setBufferCapacity(1);
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = AsyncLoggerTest::kTempLog;

  AsyncLogger::Logger::init(config);
  EXPECT_EQ(AsyncLogger::Logger::droppedCount(), 0u);

  emitBurstLogs(32, 4096);
  AsyncLogger::Logger::shutdown();

  EXPECT_GT(AsyncLogger::Logger::droppedCount(), 0u);
  AsyncLogger::Logger::resetStats();
  EXPECT_EQ(AsyncLogger::Logger::droppedCount(), 0u);
  AsyncLogger::LoggerTestPeer::resetBufferCapacity();
  std::remove(AsyncLoggerTest::kTempLog.c_str());
}

TEST(AsyncLogger, DroppedCountResetsOnReinit) {
  AsyncLogger::LoggerTestPeer::setBufferCapacity(1);
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = AsyncLoggerTest::kTempLog;

  AsyncLogger::Logger::init(config);
  emitBurstLogs(32, 4096);
  AsyncLogger::Logger::shutdown();
  ASSERT_GT(AsyncLogger::Logger::droppedCount(), 0u);

  AsyncLogger::Logger::init(config);
  EXPECT_EQ(AsyncLogger::Logger::droppedCount(), 0u);
  AsyncLogger::Logger::shutdown();
  AsyncLogger::LoggerTestPeer::resetBufferCapacity();
  std::remove(AsyncLoggerTest::kTempLog.c_str());
}
