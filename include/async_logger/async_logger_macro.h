#pragma once

#include <format>

#include "async_logger/async_logger.h"

#define LOG_DEBUG(msg) AsyncLogger::Logger::debug(msg)
#define LOG_INFO(msg) AsyncLogger::Logger::info(msg)
#define LOG_WARN(msg) AsyncLogger::Logger::warn(msg)
#define LOG_ERROR(msg) AsyncLogger::Logger::error(msg)
#define LOG_FATAL(msg) AsyncLogger::Logger::fatal(msg)

#define LOGF_DEBUG(fmt, ...) \
  AsyncLogger::Logger::debug(std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#define LOGF_INFO(fmt, ...) \
  AsyncLogger::Logger::info(std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#define LOGF_WARN(fmt, ...) \
  AsyncLogger::Logger::warn(std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#define LOGF_ERROR(fmt, ...) \
  AsyncLogger::Logger::error(std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
#define LOGF_FATAL(fmt, ...) \
  AsyncLogger::Logger::fatal(std::format(fmt __VA_OPT__(, ) __VA_ARGS__))
