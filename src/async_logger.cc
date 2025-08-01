#include "async_logger/async_logger.h"

#include <chrono>
#include <ctime>
#include <source_location>
#include <sstream>
#include <thread>

namespace AsyncLogger {

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
  buffer_.tryPush(format(lvl, loc, msg));
}

std::string Logger::format(Level lvl, const std::source_location& loc,
                           const std::string& msg) {
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

  static const char* level_names[] = {"DEBUG", "INFO", "WARN", "ERROR",
                                      "FATAL"};
  std::ostringstream oss;
  oss << "[" << level_names[static_cast<int>(lvl)] << "] ";
  oss << "[" << time_buf << "] ";
  oss << "[" << filename_only(loc.file_name()) << ":" << loc.line() << "] ";
  oss << msg;
  return oss.str();
}

void Logger::log(std::string entry) {
  if (flag_ & OutstreamFlag::out_stdout) std::cout << entry << std::endl;
  if (flag_ & OutstreamFlag::out_stderr) std::cerr << entry << std::endl;
  if (flag_ & OutstreamFlag::out_file) ofstream_ << entry << std::endl;
}

void Logger::workerLoop() {
  std::string entry;
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
