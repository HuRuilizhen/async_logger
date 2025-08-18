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
             OutstreamFlag::out_color | OutstreamFlag::mode_append;
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
  static void init(const Config& config = Config());
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

  void workerLoop();
  void enqueue(Level lvl, const std::source_location& loc,
               const std::string& msg);
  void log(const Entry& entry);
  std::string format(const Entry& entry, bool colored = false);
  static Logger& instance();

  // Config variables
  Level level_{Level::Info};
  int flag_{};
  std::ios::openmode file_mode_{};
  bool need_rotation_{};
  std::tm time_stamp_{};
  std::ofstream ofstream_{};

  // Ring buffer for storing log entries
  RingBuffer::RingBufferSemiAtomicSlot<Entry> buffer_{1024};

  // Thread and state related
  std::thread worker_;
  std::atomic<bool> running_{false};

  // Friend utils class
  friend class LoggerUtils;
};

class LoggerUtils {
 public:
  static const std::string_view getLevelString(Level lvl);
  static const std::string_view getLevelColor(Level lvl);
  static const std::tm getCurrentTime();
  static const std::tm getRoundedTime(std::tm time);
  static const bool tryUpdateTimestamp(Logger& lg);
  static const std::string getDefaultFilename();
  static const bool tryUpdateFileStream(Logger& lg);
  static constexpr std::string_view getFilenameInPath(std::string_view path);
  static const std::tm (*timeFuncPtr)();
};

}  // namespace AsyncLogger
