#include "async_logger/async_logger.h"

int main(int argc, char *argv[]) {
  AsyncLogger::Config config;
  config.filename = "test.log";
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_stdout |
                AsyncLogger::OutstreamFlag::out_stderr |
                AsyncLogger::OutstreamFlag::out_file;
  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("hello from info log!");
  AsyncLogger::Logger::shutdown();
  return 0;
}
