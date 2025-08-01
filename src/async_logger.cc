#include "async_logger/async_logger.h"

#include <chrono>
#include <ctime>
#include <source_location>
#include <sstream>
#include <thread>

namespace AsyncLogger::AnsiColor {
inline constexpr std::string_view RESET = "\033[0m";
inline constexpr std::string_view RED = "\033[31m";
inline constexpr std::string_view GREEN = "\033[32m";
inline constexpr std::string_view YELLOW = "\033[33m";
inline constexpr std::string_view BLUE = "\033[34m";
inline constexpr std::string_view MAGENTA = "\033[35m";
inline constexpr std::string_view CYAN = "\033[36m";
inline constexpr std::string_view GRAY = "\033[90m";
inline constexpr std::string_view BG_RED = "\033[41m";
}  // namespace AsyncLogger::AnsiColor

namespace AsyncLogger {

inline std::string_view getLevelString(Level lvl) {
  switch (lvl) {
    case Level::Debug:
      return "DEBUG";
    case Level::Info:
      return "INFO";
    case Level::Warn:
      return "WARN";
    case Level::Error:
      return "ERROR";
    case Level::Fatal:
      return "FATAL";
    default:
      return "UNKNOWN";
  }
}

inline std::string_view getLevelColor(Level lvl) {
  switch (lvl) {
    case Level::Debug:
      return AnsiColor::CYAN;
    case Level::Info:
      return AnsiColor::GREEN;
    case Level::Warn:
      return AnsiColor::YELLOW;
    case Level::Error:
      return AnsiColor::RED;
    case Level::Fatal:
      return AnsiColor::BG_RED;
    default:
      return AnsiColor::RESET;
  }
}

constexpr std::string_view filename_only(std::string_view path) {
  auto pos = path.find_last_of("/\\");
  return pos == std::string_view::npos ? path : path.substr(pos + 1);
}

void Logger::init(const Config& config) {
  auto& lg = instance();
  lg.level_ = config.level;
  lg.flag_ = config.flag;
  if (config.flag & OutstreamFlag::out_file)
    lg.ofstream_.open(config.filename, std::ios::out | std::ios::app);
  lg.running_ = true;
  lg.worker_ = std::thread(&Logger::workerLoop, &lg);
}

void Logger::shutdown() {
  auto& lg = instance();
  lg.running_ = false;
  if (lg.worker_.joinable()) lg.worker_.join();
  if (lg.ofstream_.is_open()) lg.ofstream_.close();
}

void Logger::debug(const std::string& msg, const std::source_location& loc) {
  instance().enqueue(Level::Debug, loc, msg);
}
void Logger::info(const std::string& msg, const std::source_location& loc) {
  instance().enqueue(Level::Info, loc, msg);
}
void Logger::warn(const std::string& msg, const std::source_location& loc) {
  instance().enqueue(Level::Warn, loc, msg);
}
void Logger::error(const std::string& msg, const std::source_location& loc) {
  instance().enqueue(Level::Error, loc, msg);
}
void Logger::fatal(const std::string& msg, const std::source_location& loc) {
  instance().enqueue(Level::Fatal, loc, msg);
}

Logger& Logger::instance() {
  static Logger lg;
  return lg;
}

Logger::~Logger() {
  if (running_) shutdown();
}

void Logger::enqueue(Level lvl, const std::source_location& loc,
                     const std::string& msg) {
  if (lvl < level_) return;
  buffer_.tryPush({lvl, loc, msg});
}

std::string Logger::format(const Entry& entry, bool colored) {
  auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &tt);
#else
  localtime_r(&tt, &tm);
#endif
  char time_buf[20];
  std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm);

  std::ostringstream oss;
  if (flag_ & colored) oss << getLevelColor(entry.lvl);
  oss << "[" << getLevelString(entry.lvl) << "]";
  if (flag_ & colored) oss << AnsiColor::RESET;
  oss << " [" << time_buf << "] ";
  oss << "[" << filename_only(entry.loc.file_name()) << ":" << entry.loc.line()
      << "] ";
  oss << entry.msg;
  return oss.str();
}

void Logger::log(const Entry& entry) {
  if (flag_ & OutstreamFlag::out_stdout)
    std::cout << format(entry, flag_ & out_color) << std::endl;
  if (flag_ & OutstreamFlag::out_stderr)
    std::cerr << format(entry, flag_ & out_color) << std::endl;
  if (flag_ & OutstreamFlag::out_file) ofstream_ << format(entry) << std::endl;
}

void Logger::workerLoop() {
  Entry entry;
  while (running_) {
    if (buffer_.tryPop(entry)) {
      Logger::log(entry);
    } else {
      std::this_thread::yield();
    }
  }
  while (buffer_.tryPop(entry)) Logger::log(entry);
}

}  // namespace AsyncLogger
