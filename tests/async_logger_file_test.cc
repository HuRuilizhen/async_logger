#include <gtest/gtest.h>

#include <chrono>
#include <ctime>
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
  // Init with defaut config value
  AsyncLogger::Logger::init(AsyncLogger::Config());
  AsyncLogger::Logger::info("test");
  AsyncLogger::Logger::shutdown();

  std::stringstream filename;
  static const std::string ext = ".log";
  std::tm tm = AsyncLogger::LoggerUtils::getCurrentTime();
  char time_buf[20];
  std::strftime(time_buf, sizeof(time_buf), "%Y-%m-%d", &tm);
  filename << static_cast<std::string>(time_buf) << ext;

  std::string content = readFile(filename.str());
  EXPECT_NE(content.find("test"), std::string::npos);

  std::remove(filename.str().c_str());
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
