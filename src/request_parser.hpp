#pragma once

#include "request.hpp"
#include "response.hpp"
#include "option.hpp"
#include "result_type.hpp"
#include "uri_parser.hpp"
#include "multipart_form_data_parser.hpp"

#include <tuple>
#include <functional>

namespace https_server {

class RequestParser {
public:
    RequestParser(const Option& opt);

    // 解析一个给定范围的字符串
    // 返回值是一个包含解析结果和最后解析位置的元组
    std::tuple<ResultType, char*> parse(Request& req,
            Response& res, char* begin, char* end);

    // 重置当前解析状态
    void reset();

    // 根据分隔符切割字符串
    // s: 需要切割的字符串
    // d: 分割符
    // func: 每切割一个字符串都将调用这个函数
    static void split(const std::string& s, const std::string& d, 
        std::function<void(const std::string&)> func);

private:
    // 请求体的大小
    int content_size_;

    // 配置选项
    const Option& opt_;

    // uri解析器
    UriParser uri_parser_;

    // 表单数据解析器
    MultipartFormDataParser multipart_form_data_parser_;

    // 解析范围请求的头部
    // ranges: 输入输出参数
    bool parseRangeHeader(const std::string& s, Ranges& ranges);

    // 解析分界符
    bool parseMultipartBoundary(const std::string &content_type,
                        std::string &boundary);

    // 解析单个字符
    ResultType consume(Request& req, Response& res, char input);

    // 检查是否为HTTP字符
    static bool is_char(int c);

    // 检查是否为HTTP控制符
    static bool is_ctl(int c);

    // 检查是否为HTTP特殊符号
    static bool is_tspecial(int c);

    // 检查是否是数字
    static bool is_digit(int c);

    // 当前的解析状态
    enum ParserState {
        method_start,
        method,
        uri,
        http_version_h,
        http_version_t_1,
        http_version_t_2,
        http_version_p,
        http_version_slash,
        http_version_major_start,
        http_version_major,
        http_version_minor_start,
        http_version_minor,
        expecting_newline_1,
        header_line_start,
        header_lws,
        header_name,
        space_before_header_value,
        header_value,
        expecting_newline_2,
        expecting_newline_3,
        body_content
    } parser_state_;
};

} // namespace https_server