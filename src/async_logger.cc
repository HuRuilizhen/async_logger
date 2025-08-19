#include "async_logger/async_logger.h"

#include <atomic>
#include <chrono>
#include <ctime>
#include <fstream>
#include <mutex>
#include <ostream>
#include <source_location>
#include <sstream>
#include <string>
#include <string_view>
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

const std::string_view LoggerUtils::getLevelString(Level lvl) {
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

const std::string_view LoggerUtils::getLevelColor(Level lvl) {
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

const std::tm LoggerUtils::getCurrentTime() {
  auto now = std::chrono::system_clock::now();
  auto tt = std::chrono::system_clock::to_time_t(now);
  std::tm tm{};
#ifdef _WIN32
  localtime_s(&tm, &tt);
#else
  localtime_r(&tt, &tm);
#endif
  return tm;
}

const std::tm (*LoggerUtils::timeFuncPtr)() = &getCurrentTime;

const std::tm LoggerUtils::getRoundedTime(std::tm time) {
  std::tm rounded_time = time;
  rounded_time.tm_hour = 0;
  rounded_time.tm_min = 0;
  rounded_time.tm_sec = 0;
  return rounded_time;
}

const bool LoggerUtils::tryUpdateTimestamp(Logger& lg) {
  std::tm cur_time = timeFuncPtr();
  if (cur_time.tm_year == lg.time_stamp_.tm_year &&
      cur_time.tm_mon == lg.time_stamp_.tm_mon &&
      cur_time.tm_mday == lg.time_stamp_.tm_mday)
    return false;
  lg.time_stamp_ = getRoundedTime(cur_time);
  return true;
}

const std::string LoggerUtils::getDefaultFilename() {
  std::tm time = timeFuncPtr();
  char filename[20];
  std::strftime(filename, sizeof(filename), "%Y-%m-%d.log", &time);
  return filename;
}

const bool LoggerUtils::tryUpdateFileStream(Logger& lg) {
  if (tryUpdateTimestamp(lg)) {
    lg.ofstream_.flush();
    lg.ofstream_.close();
    lg.ofstream_.open(getDefaultFilename(), lg.file_mode_);
    if (!lg.ofstream_.is_open()) {
      std::cerr << "Failed to open file: " << getDefaultFilename() << std::endl;
      exit(1);
    }
  }
  return false;
}

constexpr std::string_view LoggerUtils::getFilenameInPath(
    std::string_view path) {
  auto pos = path.find_last_of("/\\");
  return pos == std::string_view::npos ? path : path.substr(pos + 1);
}

void Logger::loadConfig(const Config& config) {
  level_.store(config.level, std::memory_order_relaxed);
  flag_ = config.flag;

  if (config.flag & OutstreamFlag::out_file) {
    file_mode_ = std::ios::out;
    if (config.flag & OutstreamFlag::mode_append)
      file_mode_ |= std::ios::app;
    else
      file_mode_ |= std::ios::trunc;

    std::string filename;
    if (config.filename.empty()) {
      time_stamp_ = LoggerUtils::getRoundedTime(LoggerUtils::timeFuncPtr());
      need_rotation_ = true;
      filename = LoggerUtils::getDefaultFilename();
    } else {
      filename = config.filename;
    }

    ofstream_.open(filename.c_str(), file_mode_);
    if (!ofstream_.is_open()) {
      std::cerr << "Failed to open file: " << filename << std::endl;
      exit(1);
    }
  }
}

void Logger::init(const Config& config) {
  auto& lg = instance();
  std::lock_guard<std::mutex> lock(lg.init_mutex_);
  if (lg.has_init_.load(std::memory_order_relaxed)) return;

  lg.loadConfig(config);

  lg.running_ = true;
  lg.worker_ = std::thread(&Logger::workerLoop, &lg);
  lg.has_init_.store(true, std::memory_order_release);
}

void Logger::ensureInit() {
  // fast path
  if (has_init_.load(std::memory_order_acquire)) return;

  // slow path
  std::lock_guard<std::mutex> lock(init_mutex_);
  if (has_init_.load(std::memory_order_acquire)) return;
  loadConfig(Config());
  running_ = true;
  worker_ = std::thread(&Logger::workerLoop, this);
  has_init_.store(true, std::memory_order_release);
}

void Logger::shutdown() {
  auto& lg = instance();
  lg.running_ = false;
  if (lg.worker_.joinable()) lg.worker_.join();
  if (lg.ofstream_.is_open()) {
    lg.ofstream_.flush();
    lg.ofstream_.close();
  }
  std::lock_guard<std::mutex> lock(lg.init_mutex_);
  lg.has_init_.store(false, std::memory_order_release);
}

void Logger::debug(const std::string& msg, const std::source_location& loc) {
  auto& lg = instance();
  lg.ensureInit();
  lg.enqueue(Level::Debug, loc, msg);
}
void Logger::info(const std::string& msg, const std::source_location& loc) {
  auto& lg = instance();
  lg.ensureInit();
  lg.enqueue(Level::Info, loc, msg);
}
void Logger::warn(const std::string& msg, const std::source_location& loc) {
  auto& lg = instance();
  lg.ensureInit();
  lg.enqueue(Level::Warn, loc, msg);
}
void Logger::error(const std::string& msg, const std::source_location& loc) {
  auto& lg = instance();
  lg.ensureInit();
  lg.instance().enqueue(Level::Error, loc, msg);
}
void Logger::fatal(const std::string& msg, const std::source_location& loc) {
  auto& lg = instance();
  lg.ensureInit();
  lg.enqueue(Level::Fatal, loc, msg);
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
  if (lvl < level_.load(std::memory_order_relaxed)) return;
  buffer_.tryPush({lvl, loc, msg});
}

std::string Logger::format(const Entry& entry, bool colored) {
  std::tm tm = LoggerUtils::timeFuncPtr();
  char time_buf[20];
  std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", &tm);

  std::ostringstream oss;
  if (flag_ & colored) oss << LoggerUtils::getLevelColor(entry.lvl);
  oss << "[" << LoggerUtils::getLevelString(entry.lvl) << "]";
  if (flag_ & colored) oss << AnsiColor::RESET;
  oss << " [" << time_buf << "] ";
  oss << "[" << LoggerUtils::getFilenameInPath(entry.loc.file_name()) << ":"
      << entry.loc.line() << "] ";
  oss << entry.msg;
  return oss.str();
}

void Logger::log(const Entry& entry) {
  if (flag_ & OutstreamFlag::out_stdout)
    std::cout << format(entry, flag_ & out_color) << std::endl;
  if (flag_ & OutstreamFlag::out_stderr)
    std::cerr << format(entry, flag_ & out_color) << std::endl;
  if (flag_ & OutstreamFlag::out_file) {
    if (need_rotation_) LoggerUtils::tryUpdateFileStream(instance());
    ofstream_ << format(entry) << std::endl;
  }
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
