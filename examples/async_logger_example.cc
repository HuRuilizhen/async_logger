#include "async_logger/async_logger.h"

int main(int argc, char* argv[]) {
  AsyncLogger::Config config;
  config.filename = "test.log";
  config.level = AsyncLogger::Level::Debug;
  config.flag = AsyncLogger::OutstreamFlag::out_stdout |
                AsyncLogger::OutstreamFlag::out_stderr |
                AsyncLogger::OutstreamFlag::out_file | AsyncLogger::out_color;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::debug("hello from debug log!");
  AsyncLogger::Logger::info("hello from info log!");
  AsyncLogger::Logger::warn("hello from warn log!");
  AsyncLogger::Logger::error("hello from error log!");
  AsyncLogger::Logger::fatal("hello from fatal log!");
  AsyncLogger::Logger::shutdown();
  return 0;
}
