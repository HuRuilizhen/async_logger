#include <gtest/gtest.h>

#include <string>

#include "async_logger/async_logger.h"

namespace {
static const std::string TMPLOG = "test.log";

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

TEST(AsyncLogger, AppendFileMode) {
  // Init with mode append
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file |
                AsyncLogger::OutstreamFlag::mode_append;
  config.filename = TMPLOG;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("alpha");
  AsyncLogger::Logger::shutdown();

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("beta");
  AsyncLogger::Logger::shutdown();

  std::string content = readFile(TMPLOG);
  EXPECT_NE(content.find("alpha"), std::string::npos);
  EXPECT_NE(content.find("beta"), std::string::npos);

  std::remove(TMPLOG.c_str());
}

TEST(AsyncLogger, TruncFileMode) {
  // Init with mode append
  AsyncLogger::Config config;
  config.level = AsyncLogger::Level::Info;
  config.flag = AsyncLogger::OutstreamFlag::out_file;
  config.filename = TMPLOG;

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("alpha");
  AsyncLogger::Logger::shutdown();

  AsyncLogger::Logger::init(config);
  AsyncLogger::Logger::info("beta");
  AsyncLogger::Logger::shutdown();

  std::string content = readFile(TMPLOG);
  EXPECT_EQ(content.find("alpha"), std::string::npos);
  EXPECT_NE(content.find("beta"), std::string::npos);

  std::remove(TMPLOG.c_str());
}
