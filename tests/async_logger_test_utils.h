#pragma once

#include <ctime>
#include <fstream>
#include <iterator>
#include <string>

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
  static void setBufferCapacity(size_t capacity) {
    auto& logger = Logger::instance();
    logger.buffer_capacity_ = capacity;
  }

  static void resetBufferCapacity() {
    auto& logger = Logger::instance();
    logger.buffer_capacity_ = Logger::kDefaultQueueCapacity;
  }
};

}  // namespace AsyncLogger
