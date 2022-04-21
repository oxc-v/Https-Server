#pragma once

#include <string>

namespace https_server {
namespace mime_types {

// 根据文件扩展名转换成相应的mime类型
std::string extensionToType(const std::string& extension);

// 根据mime类型转换成相应的文件扩展名
std::string typeToExtension(const std::string& type);

} // namespace mime_types
} // namespace https_server

