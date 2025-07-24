#include "async_logger/async_logger.h"

int main(int argc, char *argv[]) {
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::stdout;
  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("hello from info log!");
  AsyncLogger::Logger::shutdown();
  return 0;
}
