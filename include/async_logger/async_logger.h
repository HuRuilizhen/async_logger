#pragma once

#include <ring_buffer/mpsc.h>

#include <atomic>
#include <fstream>
#include <source_location>
#include <string>
#include <thread>

namespace AsyncLogger {

// Supported log levels
enum class Level { Debug, Info, Warn, Error, Fatal };

enum OutstreamFlag { out_stdout = 1, out_stderr = 1 << 1, out_file = 1 << 2 };

struct Config {
  std::string filename{};
  int flag = OutstreamFlag::out_stdout;
  Level level = Level::Info;
};

class Logger {
 public:
  // Initialize logger: opens file and starts worker thread
  static void init(const Config& config);
  // Shutdown logger: stops worker and flushes remaining logs
  static void shutdown();

  // Logging API
  static void debug(
      const std::string& msg,
      const std::source_location& loc = std::source_location::current());
  static void info(const std::string& msg, const std::source_location& loc =
                                               std::source_location::current());
  static void warn(const std::string& msg, const std::source_location& loc =
                                               std::source_location::current());
  static void error(
      const std::string& msg,
      const std::source_location& loc = std::source_location::current());
  static void fatal(
      const std::string& msg,
      const std::source_location& loc = std::source_location::current());

 private:
  Logger() = default;
  ~Logger();
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  int flag_{};

  void workerLoop();
  void enqueue(Level lvl, const std::source_location& loc,
               const std::string& msg);
  void log(std::string entry);
  std::string format(Level lvl, const std::source_location& loc,
                     const std::string& msg);
  static Logger& instance();

  // Ring buffer for storing log entries
  RingBuffer::RingBufferSemiAtomicSlot<std::string> buffer_{1024};
  std::thread worker_;
  std::atomic<bool> running_{false};
  std::ofstream ofstream_;
  Level level_{Level::Info};
};

}  // namespace AsyncLogger
