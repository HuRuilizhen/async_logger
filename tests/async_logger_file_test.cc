#include <gtest/gtest.h>

#include <ctime>
#include <string>
#include <thread>

#include "async_logger/async_logger.h"

namespace {
static const std::string TMPLOG = "test.log";

std::string readFile(const std::string& filename) {
  std::ifstream ifs(filename);
  std::string content((std::istreambuf_iterator<char>(ifs)),
                      std::istreambuf_iterator<char>());
  return content;
}

const std::tm newTimeFunc() {
  std::tm time = AsyncLogger::LoggerUtils::getCurrentTime();
  time.tm_mday += 1;
  return time;
}

}  // namespace

TEST(AsyncLogger, EmptyFileName) {
  // Init with defaut config value
  AsyncLogger::Logger::init(AsyncLogger::Config());
  AsyncLogger::Logger::info("test");
  AsyncLogger::Logger::shutdown();

  std::tm tm = AsyncLogger::LoggerUtils::getCurrentTime();
  char filename[20];
  std::strftime(filename, sizeof(filename), "%Y-%m-%d.log", &tm);

  std::string content = readFile(filename);
  EXPECT_NE(content.find("test"), std::string::npos);

  std::remove(filename);
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

TEST(AsyncLogger, LogFileRotation) {
  AsyncLogger::Logger::init(AsyncLogger::Config());
  AsyncLogger::Logger::info("alpha");
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  std::string filename = AsyncLogger::LoggerUtils::getDefaultFilename();
  std::string content = readFile(filename);
  EXPECT_NE(content.find("alpha"), std::string::npos);
  std::remove(filename.c_str());

  AsyncLogger::LoggerUtils::timeFuncPtr = &newTimeFunc;
  AsyncLogger::Logger::info("beta");
  AsyncLogger::Logger::shutdown();

  filename = AsyncLogger::LoggerUtils::getDefaultFilename();
  content = readFile(filename);
  EXPECT_NE(content.find("beta"), std::string::npos);
  std::remove(filename.c_str());
}
