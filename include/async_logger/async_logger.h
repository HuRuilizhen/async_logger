#pragma once

#include <ring_buffer/mpsc.h>

#include <atomic>
#include <fstream>
#include <ostream>
#include <string>
#include <thread>

namespace AsyncLogger {

// Supported log levels
enum class Level { Debug, Info, Warn, Error, Fatal };

enum OutstreamFlag { stdout = 1, stderr = 1 << 1, file = 1 << 2 };

struct Config {
  std::string filename{};
  int flag = OutstreamFlag::stdout;
  Level level = Level::Info;
};

class Logger {
 public:
  // Initialize logger: opens file and starts worker thread
  static void init(const Config& config);
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

  int flag_{};

  void workerLoop();
  void enqueue(Level lvl, const std::string& msg);
  void log(std::string entry);
  std::string format(Level lvl, const std::string& msg);
  static Logger& instance();

  // Ring buffer for storing log entries
  RingBuffer::RingBufferSemiAtomicSlot<std::string> buffer_{1024};
  std::thread worker_;
  std::atomic<bool> running_{false};
  std::ofstream ofstream_;
  Level level_{Level::Info};
};

}  // namespace AsyncLogger
