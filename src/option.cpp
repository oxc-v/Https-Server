#include "option.hpp"

using std::string;

namespace https_server {

string Option::crtFilePath() const
{
    return crt_file_path_;
}

void Option::setCrtFilePath(const string& path)
{
    crt_file_path_ = path;
}

string Option::privateKeyFilePath() const
{
    return private_key_file_path_;
}

void Option::setPrivateKeyFilePath(const string& path)
{
    private_key_file_path_ = path;
}

string Option::privateKeyPwd() const
{
    return private_key_pwd_;
}

void Option::setPrivateKeyPwd(const string& pwd)
{
    private_key_pwd_ = pwd;
}

std::size_t Option::connectionTimeout() const 
{
    return connection_timeout_;
}

void Option::setConnectionTimeout(const std::size_t timeout)
{
    connection_timeout_ = timeout;
}

void Option::setUriMaxLength(const std::size_t l)
{
    uri_max_length_ = l;
}

std::size_t Option::uriMaxLength() const
{
    return uri_max_length_;
}

void Option::setRequestMaxLength(const std::size_t l)
{
    request_max_length_ = l;
}

std::size_t Option::requestMaxLength() const
{
    return request_max_length_;
}

EncodingType Option::encodingType() const
{
    return encoding_type_;
}

void Option::setEncodingType(const EncodingType& e)
{
    encoding_type_ = e;
}

} // namespace https_server