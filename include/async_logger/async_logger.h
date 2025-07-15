#pragma once

#include <ring_buffer/internal/semiatomic_slot.h>

#include <atomic>
#include <fstream>
#include <string>
#include <thread>

#include "ring_buffer/mpsc.h"

namespace AsyncLogger {

// Supported log levels
enum class Level { Debug, Info, Warn, Error, Fatal };

class Logger {
 public:
  // Initialize logger: opens file and starts worker thread
  static void init(const std::string& filename, Level level = Level::Info);
  // Shutdown logger: stops worker and flushes remaining logs
  static void shutdown();

  // Logging API
  static void debug(const std::string& msg);
  static void info(const std::string& msg);
  static void warn(const std::string& msg);
  static void error(const std::string& msg);
  static void fatal(const std::string& msg);

 private:
  Logger() = default;
  ~Logger();
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void workerLoop();
  void enqueue(Level lvl, const std::string& msg);
  std::string format(Level lvl, const std::string& msg);
  static Logger& instance();

  // Ring buffer for storing log entries
  RingBuffer::RingBufferSemiAtomicSlot<std::string> buffer_{1024};
  std::thread worker_;
  std::atomic<bool> running_{false};
  std::ofstream output_;
  Level level_{Level::Info};
};

}  // namespace AsyncLogger
