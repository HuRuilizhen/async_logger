#pragma once

#include <ring_buffer/mpsc_ring_buffer.h>

#include <atomic>
#include <condition_variable>
#include <ctime>
#include <fstream>
#include <memory>
#include <mutex>
#include <source_location>
#include <stdexcept>
#include <string>
#include <thread>

namespace AsyncLogger {

// Supported log levels.
enum class Level { Debug, Info, Warn, Error, Fatal };

// Worker wait strategy when the queue is empty.
enum class WaitStrategy { Blocking, Yielding };

// Output control flags.
enum OutstreamFlag {
  out_stdout = 1,       // log to std out
  out_stderr = 1 << 1,  // log to std error
  out_file = 1 << 2,    // log to given file
  out_color = 1 << 3,   // enable colored level, only work on std out
  mode_append = 1 << 4  // open file in append mode
};

// Logger configuration applied during init().
struct Config {
  std::string filename{};
  int flag = OutstreamFlag::out_stdout | OutstreamFlag::out_file |
             OutstreamFlag::out_color | OutstreamFlag::mode_append;
  Level level = Level::Info;
  WaitStrategy wait_strategy = WaitStrategy::Blocking;
};

struct Entry {
  Level lvl = Level::Info;
  std::tm timestamp{};
  std::source_location loc;
  std::string msg;
};

class FileOpenError : public std::runtime_error {
 public:
  explicit FileOpenError(const std::string& filename);
};

class Logger {
 public:
  // Initialize the singleton logger, start the worker, and open the file sink
  // when enabled.
  static void init(const Config& config = Config());
  // Stop the worker and flush any remaining queued entries.
  static void shutdown();
  // Number of log entries dropped because the async queue was full during the
  // current logger lifetime.
  static size_t droppedCount();
  // Reset drop accounting without changing the active logger configuration.
  static void resetStats();

  // Logging API.
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
  Logger();
  ~Logger();
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  void workerLoop();
  void enqueue(Level lvl, const std::source_location& loc,
               const std::string& msg);
  void log(const Entry& entry);
  std::string format(const Entry& entry, bool colored = false);
  void notifyWorkerForEnqueuedEntry();
  void markEntryDequeued();
  void waitForWork();
  void resetRuntimeState();
  static Logger& instance();

  // Initialize private method
  void loadConfig(const Config& config);
  void ensureInit();

  // Config variables
  std::mutex init_mutex_;
  std::atomic<bool> has_init_{};
  std::atomic<Level> level_{Level::Info};
  int flag_{};
  std::ios::openmode file_mode_{};
  bool need_rotation_{};
  std::tm time_stamp_{};
  std::ofstream ofstream_{};
  WaitStrategy wait_strategy_{WaitStrategy::Blocking};

  // Ring buffer for storing log entries
  static constexpr size_t kDefaultQueueCapacity = 1024;
  size_t buffer_capacity_{kDefaultQueueCapacity};
  std::unique_ptr<RingBuffer::MPSCRingBuffer<Entry>> buffer_;
  size_t queued_entries_{0};
  std::atomic<size_t> dropped_count_{0};

  // Thread and state related
  std::condition_variable work_ready_cv_;
  std::mutex work_ready_mutex_;
  std::thread worker_;
  std::atomic<bool> running_{false};

  // Friend utils class
  friend class LoggerUtils;
  friend class LoggerTestPeer;
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
  // Injectable time provider used by tests.
  static const std::tm (*timeFuncPtr)();
};

}  // namespace AsyncLogger
