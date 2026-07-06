#pragma once

#include <chrono>
#include <ctime>
#include <fstream>
#include <iterator>
#include <string>
#include <thread>

#include "async_logger/async_logger.h"

namespace AsyncLoggerTest {

inline const std::string kTempLog = "test.log";

inline std::string readFile(const std::string& filename) {
  std::ifstream ifs(filename);
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());
}

inline std::string formatTime(const std::tm& tm) {
  char time_buf[20];
  std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm);
  return time_buf;
}

class TimeFuncGuard {
 public:
  TimeFuncGuard() : old_(AsyncLogger::LoggerUtils::timeFuncPtr) {}
  ~TimeFuncGuard() { AsyncLogger::LoggerUtils::timeFuncPtr = old_; }

 private:
  const std::tm (*old_)();
};

}  // namespace AsyncLoggerTest

namespace AsyncLogger {

class LoggerTestPeer {
 public:
  static bool waitUntilWorkerIsWaiting() {
    auto& logger = Logger::instance();
    for (int i = 0; i < 1000; ++i) {
      {
        std::lock_guard<std::mutex> lock(logger.work_ready_mutex_);
        if (logger.worker_waiting_) return true;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return false;
  }

  static void fillBufferToCapacity() {
    auto& logger = Logger::instance();
    Entry entry{Level::Info, LoggerUtils::getCurrentTime(),
                std::source_location::current(), "prefill"};
    while (logger.buffer_.tryPush(entry)) {
    }
  }
};

}  // namespace AsyncLogger
