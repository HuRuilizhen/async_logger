#include "async_logger/async_logger_macro.h"

int main(int argc, char *argv[]) {
  AsyncLogger::Logger::init(AsyncLogger::Config());
  LOG_INFO("hello from macro info log!");
  LOGF_INFO("hello from macro {} log!", "info");
  return 0;
}
