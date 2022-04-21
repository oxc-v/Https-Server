#pragma once

#include <map>
#include <string>

namespace https_server {

class Request;

class UriParser {
public:
    UriParser() = default;

    // 解析uri
    void parse(Request& req);

    // 对uri进行解码
    static bool uriDecode(const std::string& in, std::string& out);
};

} // namespace https_server