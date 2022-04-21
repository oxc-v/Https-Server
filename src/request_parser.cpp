#include "request_parser.hpp"


#include <set>
#include <exception>
#include <regex>



using std::string;
using std::tuple;
using std::set;
using std::regex;
using std::smatch;
using std::regex_match;

namespace https_server {

RequestParser::RequestParser(const Option& opt)
    : parser_state_(method_start),
      content_size_(0),
      opt_(opt) {}

void RequestParser::reset() {
    parser_state_ = method_start;
    content_size_ = 0;
    multipart_form_data_parser_.reset();
}

tuple<ResultType, char*> RequestParser::parse(Request& req,
            Response& res, char* begin, char* end)
{
    // 逐个扫描字符
    while (begin != end) {
        ResultType result = consume(req, res, *begin++);
        if (result == bad || result == good) {
            // 解析表单数据
            if (req.isMultipartFormData() && result == good) {
                string boundary;
                auto content_type = req.getHeaderValue("Content-Type");
                if (!parseMultipartBoundary(content_type, boundary)) {
                    std::make_tuple(bad, begin);
                }
                multipart_form_data_parser_.setBoundary(std::move(boundary));
                auto r = multipart_form_data_parser_.parse(req, req.body.c_str(), req.body.size());
                return std::make_tuple(r, begin);
            }

            return std::make_tuple(result, begin);
        }
    }

    return std::make_tuple(indeterminate, begin);
}

ResultType RequestParser::consume(Request& req, Response& res, char input)
{
    // 匹配当前解析状态
    switch (parser_state_) {
    case method_start:
        if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            parser_state_ = method;
            req.method.push_back(input);
            return indeterminate;
        }
    case method:
        if (input == ' ') {
            // 支持的请求方法
            static const set<std::string> methods{ "GET", "HEAD", "POST"};
            if (methods.find(req.method) == methods.end()) { 
                res.status = StatusCode::not_implemented;
                return bad; 
            }
            parser_state_ = uri;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            req.method.push_back(input);
            return indeterminate;
        }
    case uri:
        if (input == ' ') {
            string path = req.uri;
            if (!UriParser::uriDecode(path, req.uri)) {
                return bad;
            }
            uri_parser_.parse(req);
            parser_state_ = http_version_h;
            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            req.uri.push_back(input);
            if (req.uri.size() > opt_.uriMaxLength()) {
                res.status = StatusCode::uri_too_long;
                return bad;
            }
            return indeterminate;
        }
    case http_version_h:
        if (input == 'H') {
            req.http_version.push_back(input);
            parser_state_ = http_version_t_1;
            return indeterminate;
        } else{
            return bad;
        }
    case http_version_t_1:
        if (input == 'T') {
            req.http_version.push_back(input);
            parser_state_ = http_version_t_2;
            return indeterminate;
        } else{
            return bad;
        }
    case http_version_t_2:
        if (input == 'T') {
            req.http_version.push_back(input);
            parser_state_ = http_version_p;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_p:
        if (input == 'P') {
            req.http_version.push_back(input);
            parser_state_ = http_version_slash;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_slash:
        if (input == '/') {
            req.http_version.push_back(input);
            parser_state_ = http_version_major_start;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_major_start:
        if (is_digit(input)) {
            req.http_version.push_back(input);
            parser_state_ = http_version_major;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_major:
        if (input == '.') {
            req.http_version.push_back(input);
            parser_state_ = http_version_minor_start;
            return indeterminate;
        } else if (is_digit(input)) {
            req.http_version.push_back(input);
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_minor_start:
        if (is_digit(input)) {
            req.http_version.push_back(input);
            parser_state_ = http_version_minor;
            return indeterminate;
        } else {
            return bad;
        }
    case http_version_minor:
        if (input == '\r') {
            // 只支持HTTP/1.1
            if (req.http_version != "HTTP/1.1") {
                res.status = StatusCode::http_version_not_supported;
                return bad;
            }
            parser_state_ = expecting_newline_1;
            return indeterminate;
        } else if (is_digit(input)) {
            req.http_version.push_back(input);
            return indeterminate;
        } else {
            return bad;
        }
    case expecting_newline_1:
        if (input == '\n') {
            parser_state_ = header_line_start;
            return indeterminate;
        } else {
            return bad;
        }
    case header_line_start:
        if (input == '\r') {
            parser_state_ = expecting_newline_3;
            return indeterminate;
        } else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
            parser_state_ = header_lws;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            req.headers.push_back(Header());
            req.headers.back().name.push_back(tolower(input));
            parser_state_ = header_name;
            return indeterminate;
        }
    case header_lws:
        if (input == '\r') {
            parser_state_ = expecting_newline_2;
            return indeterminate;
        } else if (input == ' ' || input == '\t') {
            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            parser_state_ = header_value;
            req.headers.back().value.push_back(input);
            return indeterminate;
        }
    case header_name:
        if (input == ':') {
            parser_state_ = space_before_header_value;
            return indeterminate;
        } else if (!is_char(input) || is_ctl(input) || is_tspecial(input)) {
            return bad;
        } else {
            req.headers.back().name.push_back(tolower(input));
            return indeterminate;
        }
    case space_before_header_value:
        if (input == ' ') {
            parser_state_ = header_value;
            return indeterminate;
        } else {
            return bad;
        }
    case header_value:
        if (input == '\r') {
            parser_state_ = expecting_newline_2;
            return indeterminate;
        } else if (is_ctl(input)) {
            return bad;
        } else {
            req.headers.back().value.push_back(input);
            return indeterminate;
        }
    case expecting_newline_2:
        if (input == '\n') {
            parser_state_ = header_line_start;
            return indeterminate;
        } else {
            return bad;
        }
    case expecting_newline_3:
        if (input == '\n') {
            if (req.method == "POST") {
                // 必须携带Content-Length
                if (!req.hasHeader("Content-Length")) {
                    res.status = StatusCode::length_required;
                    return bad;
                } else {
                    auto value = req.getHeaderValue("Content-Length");
                    try {
                        content_size_ = std::stoi(value);
                    } catch (const std::exception& e) {
                        return bad;
                    }
                    if (content_size_ == 0) {
                        return good;
                    } else if (content_size_ > opt_.requestMaxLength()) {
                        res.status = StatusCode::payload_too_large;
                        return bad;
                    } else {
                        parser_state_ = body_content;
                        return indeterminate;
                    }
                }
            }

            if (req.hasHeader("Range")) {
                if (!parseRangeHeader(req.getHeaderValue("Range"), req.ranges))
                    return bad;
            }
            return good;
        } else {
            return bad;
        }
    case body_content:
        --content_size_;
        req.body.push_back(input);
        if(content_size_ == 0)
            return good;
        else
            return indeterminate;
    default:
        return bad;
    }
}

void RequestParser::split(const string& s, const string& d,
    std::function<void(const string&)> func)
{
    std::size_t rpos = 0;
    std::size_t epos = 0;
    string token;

    while ((epos = s.find(d, rpos)) != string::npos) {
        token = s.substr(rpos, epos);
        func(token);
        rpos = epos + d.length();
        // 测试
        // std::cout << token << std::endl;
    }

    if (rpos < s.length() - 1) {
        token = s.substr(rpos, epos);
        func(token);
    }
}

bool RequestParser::parseRangeHeader(const string& s, 
    Ranges& ranges)
{
    static auto re_first_range = regex(R"(bytes=(\d*-\d*(?:,\s*\d*-\d*)*))");
    smatch m_first_range;
    if (regex_match(s, m_first_range, re_first_range)) {
        bool all_valid_ranges = true;
        split(m_first_range[1], ",", [&](const string& token) {
            // 前面的范围错误，停止解析后面的范围
            if (!all_valid_ranges) return;

            // 解析范围
            static auto re_another_range = regex(R"(\s*(\d*)-(\d*))");
            smatch m_another_range;
            if (regex_match(token, m_another_range, re_another_range)) {
                try {
                    ssize_t first = -1;
                    auto first_str = m_another_range[1].str();
                    if (!first_str.empty()) {
                        first = static_cast<ssize_t>(std::stoll(first_str));
                    }

                    ssize_t last = -1;
                    auto last_str = m_another_range[2].str();
                    if (!last_str.empty()) {
                        last = static_cast<ssize_t>(std::stoll(last_str));
                    }

                    // 范围错误
                    if (first != -1 && last != -1 && first > last) {
                        all_valid_ranges = false;
                        return;
                    }

                    ranges.emplace_back(std::make_pair(first, last));
                } catch (const std::exception&) {
                    all_valid_ranges = false;
                    return;
                }
            }
        });
        return all_valid_ranges;
    }

    return false;
}

bool RequestParser::parseMultipartBoundary(const string& content_type,
                        std::string& boundary)
{
    auto pos = content_type.find("boundary=");
    if (pos == string::npos) { 
        return false; 
    }
    boundary = content_type.substr(pos + 9);
    if (boundary.length() >= 2 && boundary.front() == '"' &&
        boundary.back() == '"') {
        boundary = boundary.substr(1, boundary.size() - 2);
    }
    return !boundary.empty();
}

bool RequestParser::is_char(int c)
{
    return c >= 0 && c <= 127;
}

bool RequestParser::is_ctl(int c)
{
    return (c >= 0 && c <= 31) || (c == 127);
}

bool RequestParser::is_tspecial(int c)
{
    switch (c)
    {
    case '(': case ')': case '<': case '>': case '@':
    case ',': case ';': case ':': case '\\': case '"':
    case '/': case '[': case ']': case '?': case '=':
    case '{': case '}': case ' ': case '\t':
        return true;
    default:
        return false;
    }
}

bool RequestParser::is_digit(int c)
{
    return c >= '0' && c <= '9';
}

} // namespace https_server