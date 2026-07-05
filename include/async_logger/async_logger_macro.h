#pragma once

#include <format>
#include <string>
#include <string_view>
#include <utility>

#include "async_logger/async_logger.h"

namespace AsyncLogger::Detail {

inline std::string formatLogMessage(std::string_view msg) {
  return std::string(msg);
}

template <typename... Args>
inline std::string formatLogMessage(std::format_string<Args...> fmt,
                                    Args&&... args) {
  return std::format(fmt, std::forward<Args>(args)...);
}

}  // namespace AsyncLogger::Detail

#define LOG_DEBUG(msg) AsyncLogger::Logger::debug(msg)
#define LOG_INFO(msg) AsyncLogger::Logger::info(msg)
#define LOG_WARN(msg) AsyncLogger::Logger::warn(msg)
#define LOG_ERROR(msg) AsyncLogger::Logger::error(msg)
#define LOG_FATAL(msg) AsyncLogger::Logger::fatal(msg)

#define LOGF_DEBUG(...) \
  AsyncLogger::Logger::debug(AsyncLogger::Detail::formatLogMessage(__VA_ARGS__))
#define LOGF_INFO(...) \
  AsyncLogger::Logger::info(AsyncLogger::Detail::formatLogMessage(__VA_ARGS__))
#define LOGF_WARN(...) \
  AsyncLogger::Logger::warn(AsyncLogger::Detail::formatLogMessage(__VA_ARGS__))
#define LOGF_ERROR(...) \
  AsyncLogger::Logger::error(AsyncLogger::Detail::formatLogMessage(__VA_ARGS__))
#define LOGF_FATAL(...) \
  AsyncLogger::Logger::fatal(AsyncLogger::Detail::formatLogMessage(__VA_ARGS__))
