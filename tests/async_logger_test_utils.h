#pragma once

#include <fstream>
#include <iterator>
#include <string>

namespace AsyncLoggerTest {

inline const std::string kTempLog = "test.log";

inline std::string readFile(const std::string& filename) {
  std::ifstream ifs(filename);
  return std::string((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());
}

}  // namespace AsyncLoggerTest
