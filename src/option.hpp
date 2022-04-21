#pragma once

#include "encoding_type.hpp"

#include <string>

namespace https_server {

class Option
{
private:
    // 证书文件路径
    std::string crt_file_path_ = "/home/oxc/ssl/minica/localhost/localhost.pem";

    // 私钥文件路径
    std::string private_key_file_path_ = "/home/oxc/ssl/minica/localhost/localhost.pem";

    // 私钥密码
    std::string private_key_pwd_ = "";

    // 0表示永不超时
    std::size_t connection_timeout_ = 0;

    // 服务器能接受的最大uri长度
    std::size_t uri_max_length_ = 1024;

    // 服务器能接受的最大request长度
    std::size_t request_max_length_ = 8388608;

    // 编码类型
    EncodingType encoding_type_ = EncodingType::Brotli;

public:
    Option() = default;

    std::string crtFilePath() const;
    void setCrtFilePath(const std::string& path);

    std::string privateKeyFilePath() const;
    void setPrivateKeyFilePath(const std::string& path);

    std::string privateKeyPwd() const;
    void setPrivateKeyPwd(const std::string& pwd);

    std::size_t connectionTimeout() const;
    void setConnectionTimeout(const std::size_t timeout);

    std::size_t uriMaxLength() const;
    void setUriMaxLength(const std::size_t l);

    std::size_t requestMaxLength() const;
    void setRequestMaxLength(const std::size_t l);

    EncodingType encodingType() const;
    void setEncodingType(const EncodingType& e);
};

} // namespace https_server