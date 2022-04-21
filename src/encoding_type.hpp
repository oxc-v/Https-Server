#pragma once

namespace https_server {

class Request;
class Response;
class Option;

// 编码类型
enum class EncodingType
{
    None = 0,
    Gzip,
    Brotli
};

namespace encoding_type {

// 根据response Content-Type以及request Accept-Encoding
// 返回相应的编码类型
EncodingType encodingType(const Request& req, const Response& res, 
                    const Option& opt);

} // namespace encoding_type

} // namespace https_server