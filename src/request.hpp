#pragma once

#include "uri_parser.hpp"
#include "header.hpp"
#include "multipart_form_data.hpp"

#include <string>
#include <vector>
#include <map>

namespace https_server {

using Range = std::pair<ssize_t, ssize_t>;
using Ranges = std::vector<Range>;
using Params = std::map<std::string, std::string>;

struct Request 
{
    // HTTP方法
    std::string method;

    // HTTP版本
    std::string http_version;

    // 查询参数
    Params params;

    std::string uri;

    // 服务路径
    // 如：/func/oxc/text.html, path = /func
    std::string path;

    // 文件路径
    // 如：/func/oxc/text.html, unresolved_path = /oxc/text.html
    std::string unresolved_path;

    // HTTP头部
    std::vector<Header> headers;
    
    // HTTP Content
    std::string body;

    // 远程地址
    std::string remote_addr;

    // 表单数据
    using MultipartFormDataMap = std::multimap<std::string, MultipartFormData>;
    MultipartFormDataMap files;

    // 范围请求
    Ranges ranges;

    // 根据key判断files中是否包含某个文件
    bool hasFile(const std::string& key) const;

    // 根据key获取files中的某个文件
    MultipartFormData getFileValue(const std::string& key) const;

    // 根据name从headers中返回对应的value,
    // 如果找不到对应的value，则返回空字符串""
    // name大小写不敏感
    std::string getHeaderValue(const std::string& name) const;

    // 判断请求体中是否包含某个头部信息
    // name大小写不敏感
    bool hasHeader(const std::string& name) const;

    // 根据key判断request是否包含某个查询参数
    bool hasParam(const std::string& key) const;

    // 根据key返回对应的参数值
    // 如果不存在该key，返回string()
    std::string getParamValue(const std::string& key) const;

    // 判断是否为表单数据
    bool isMultipartFormData() const;
};

} // namespace https_server
