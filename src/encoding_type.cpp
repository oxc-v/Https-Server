#include "encoding_type.hpp"
#include "request.hpp"
#include "response.hpp"
#include "option.hpp"

#include <string>

using std::string;

namespace https_server {
namespace encoding_type {

// 字符串映射
constexpr unsigned int str2tag_core(const char* s, std::size_t l,
                    unsigned int h) {
    return (l == 0) ? h
            : str2tag_core(s + 1, l - 1,
              (h * 33) ^ static_cast<unsigned char>(*s));
}

unsigned int str2tag(const std::string& s) {
    return str2tag_core(s.data(), s.size(), 0);
}

constexpr unsigned int operator"" _t(const char *s, size_t l) {
    return str2tag_core(s, l, 0);
}

// 判断content-type是否支持压缩
bool canCompressContentType(const string &content_type) {

    auto tag = str2tag(content_type);

    // 支持压缩的类型
    switch (tag) {
    case "image/svg+xml"_t:
    case "application/javascript"_t:
    case "application/json"_t:
    case "application/xml"_t:
    case "application/protobuf"_t:
    case "application/xhtml+xml"_t: return true;

    default:
        return !content_type.rfind("text/", 0) && tag != "text/event-stream"_t;
    }
}

EncodingType encodingType(const Request& req, const Response& res, const Option& opt) {
    auto type = opt.encodingType();
    if (type == EncodingType::None) { return EncodingType::None; }

    auto ret =
        canCompressContentType(res.getHeaderValue("Content-Type"));
    if (!ret) { return EncodingType::None; }

    const auto s = req.getHeaderValue("Accept-Encoding");

    // TODO: 'Accept-Encoding' has br, not br;q=0
    ret = s.find("br") != std::string::npos;
    if (ret && type == EncodingType::Brotli) { return EncodingType::Brotli; }

    // TODO: 'Accept-Encoding' has gzip, not gzip;q=0
    ret = s.find("gzip") != std::string::npos;
    if (ret && type == EncodingType::Gzip) { return EncodingType::Gzip; }

    return EncodingType::None;
}

} // namespace encoding_type
} // namespace https_server
