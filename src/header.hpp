#pragma once

#include <string>

namespace https_server {

// HTTP头部结构
struct Header {
    std::string name;

    std::string value;
};

} // namespace https_server