#pragma once

#include "header.hpp"
#include "status_code.hpp"
#include "data_sink.hpp"

#include <string>
#include <vector>
#include <functional>

namespace https_server {

using ContentProvider =
    std::function<void(std::size_t offset, std::size_t length, DataSink& sink)>;

using ContentProviderWithoutLength =
    std::function<void(DataSink &sink)>;

struct Response {
    // 响应消息对应的状态码
    StatusCode status = StatusCode::bad_request;

    // 响应的具体内容
    std::string body;

    // 响应消息的全部头部信息
    std::vector<Header> headers;

    // 设置响应的头部信息
    // 如果headers_中已经存在一个相同的头部，则修改当前头部信息为传入值
    void setHeader(const std::string& name, const std::string& value);

    // 在现有的头部追加值
    // 如: Transfer-Encoding: chunked, gzip, br
    // 每一个值以 , 分隔
    // 如果不存在header，不进行追加操作
    void appendHeader(const std::string& name, const std::string& value);

    // 根据name从headers中返回对应的value,
    // 如果找不到对应的value，则返回空字符串""
    std::string getHeaderValue(const std::string& name) const;

    // 判断请求体中是否包含某个头部信息
    // name大小写不敏感
    bool hasHeader(const std::string& name) const;

    // 设置响应内容
    void setContent(const std::string& data, 
                const std::string& content_type);

    // 返回一个简单的响应消息
    static Response stockResponse(const StatusCode& status);

    // 使用数据供应器发送数据
    // len: 数据长度
    // content_type: 数据类型
    // provider: 数据供应器
    void setContentProvider(std::size_t len, const std::string& content_type, 
        ContentProvider provider);
    
    // 分块传输供应器
    void setChunkedContentProvider(const std::string& content_type, 
            ContentProviderWithoutLength provider);

    // 数据长度
    std::size_t content_len_;

    // 指定长度的数据供应器
    ContentProvider content_provider_;

    // 不指定长度的数据供应器
    ContentProviderWithoutLength content_provider_without_length_;
};

} // namespace https_server