#pragma once

#include <ring_buffer/mpsc.h>

#include <atomic>
#include <fstream>
#include <source_location>
#include <string>
#include <string_view>
#include <thread>

namespace AsyncLogger {

// Supported log levels
enum class Level { Debug, Info, Warn, Error, Fatal };

// Supported ostream control flag
enum OutstreamFlag {
  out_stdout = 1,       // log to std out
  out_stderr = 1 << 1,  // log to std error
  out_file = 1 << 2,    // log to given file
  out_color = 1 << 3,   // enabel colored level, only work on std out
  mode_append = 1 << 4  // open file in append mode
};

struct Config {
  std::string filename{};
  int flag = OutstreamFlag::out_stdout | OutstreamFlag::out_file |
             OutstreamFlag::out_color;
  Level level = Level::Info;
};

struct Entry {
  Level lvl = Level::Info;
  std::source_location loc;
  std::string msg;
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
  void log(const Entry& entry);
  std::string format(const Entry& entry, bool colored = false);
  static Logger& instance();

  // Ring buffer for storing log entries
  RingBuffer::RingBufferSemiAtomicSlot<Entry> buffer_{1024};

  std::thread worker_;
  std::atomic<bool> running_{false};
  std::ofstream ofstream_;
  Level level_{Level::Info};
};

}  // namespace AsyncLogger
