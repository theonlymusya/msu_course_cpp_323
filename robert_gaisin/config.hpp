#pragma once
#include <string>

namespace config {
constexpr const char* kTempDirectoryPath = "./temp/";
std::string log_file_path() {
  return kTempDirectoryPath;
}
}  // namespace config
