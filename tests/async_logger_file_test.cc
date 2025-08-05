#include <gtest/gtest.h>

#include <string>

#include "async_logger/async_logger.h"

namespace {

std::string readFile(const std::string& filename) {
  std::ifstream ifs(filename);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  return content;
}
}  // namespace

TEST(AsyncLogger, EmptyFileName) {
  // Init without given filename
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Warn;
  config.flag = AsyncLogger::OutstreamFlag::out_file;

  // Should exit with error
  EXPECT_EXIT(AsyncLogger::Logger::init(config);
              , ::testing::ExitedWithCode(1), "Failed to open file");
}
