#include "multipart_form_data_parser.hpp"
#include "request.hpp"

#include <regex>


using std::regex;
using std::regex_match;
using std::smatch;
using std::string;

namespace https_server {

MultipartFormDataParser::MultipartFormDataParser()
    : cur_pos_(0),
      end_pos_(0),
      state_(initial_boundary) {}

void MultipartFormDataParser::setBoundary(string&& boundary)
{
    boundary_ = boundary;
}

ResultType MultipartFormDataParser::parse(
    Request& req, const char* buf, std::size_t n)
{
    // 提取name、filename
    static const regex re_content_disposition(
        R"~(^Content-Disposition:\s*form-data;\s*name="(.*?)"(?:;\s*filename="(.*?)")?(?:;\s*filename\*=\S+)?\s*$)~",
        std::regex_constants::icase);
    // 提取content-type
    static const regex re_content_type(
        R"(^content-type:\s*(.*)\s*)",
        std::regex_constants::icase);

    static const string dash = "--";
    static const string crlf = "\r\n";

    // 往buf_追加数据
    appendBuf(buf, n);

    while (unParseBufSize() > 0) {
        switch (state_) {
        case initial_boundary: {
            auto pattern = dash + boundary_ + crlf;
            if (pattern.size() > unParseBufSize()) { 
                return indeterminate; 
            }
            if (!compareBufSubStr(pattern)) { 
                return bad; 
            }
            moveCurPos(pattern.size());
            state_ = new_entry;
            break;
        }
        case new_entry: {
            clearFileInfo();
            state_ = headers;
            break;
        }
        case headers: {
            auto pos = bufFind(crlf);
            while (pos < unParseBufSize()) {
                // 空行
                if (pos == 0) {
                    moveCurPos(crlf.size());
                    state_ = body;
                    break;
                }

                static const string header_name = "Content-Type:";
                const auto header = bufHead(pos);
                if (compareWithCaseIgnore(header, header_name)) {
                    // 提取content-type
                    smatch sm;
                    if (regex_match(header, sm, re_content_type)) {
                        file_.content_type = sm[1];
                    } else {
                        return bad;
                    }
                } else {
                    // 提取文件名、文件类型
                    smatch m;
                    if (regex_match(header, m, re_content_disposition)) {
                        file_.name = m[1];
                        file_.filename = m[2];
                    } else {
                        return bad;
                    }
                }
                
                // 移动当前解析位置
                moveCurPos(pos + crlf.size());
                // 查找下一个header
                pos = bufFind(crlf);
            }

            // 数据不完整
            if (state_ != body) { 
                return indeterminate;
            }
            break;
        }
        case body: {
            auto pattern = crlf + dash + boundary_;
            if (pattern.size() > unParseBufSize()) { 
                return indeterminate;  
            }
            auto pos = bufFind(pattern);
            if (pos < unParseBufSize()) {
                file_.content = bufHead(pos);
                moveCurPos(pos + pattern.size());
                req.files.emplace(file_.name, file_);
                state_ = boundary;
            } else {
                return indeterminate;
            }
            break;
        }
        case boundary: {
            if (crlf.size() > unParseBufSize()) { 
                return indeterminate; 
            }
            if (compareBufSubStr(crlf)) {
                moveCurPos(crlf.size());
                state_ = new_entry;
            } else {
                auto pattern = dash + crlf;
                if (pattern.size() > unParseBufSize()) { 
                    return indeterminate; 
                }
                if (compareBufSubStr(pattern)) {
                    moveCurPos(pattern.size());
                    return good;
                } else {
                    return indeterminate; 
                }
            }
            break;
        }
        default:
            return bad;
        }
    }

    return indeterminate;
}

void MultipartFormDataParser::reset()
{
    state_ = initial_boundary;
    cur_pos_ = 0;
    end_pos_ = 0;
    buf_.clear();
    boundary_.clear();
    clearFileInfo();
}

std::size_t MultipartFormDataParser::unParseBufSize() const
{
    return end_pos_ - cur_pos_;
}

void MultipartFormDataParser::clearFileInfo()
{
    file_.content.clear();
    file_.content_type.clear();
    file_.name.clear();
    file_.filename.clear();
}

bool MultipartFormDataParser::compareWithCaseIgnore(
    const string& a, const string& b ) const
{
    if (a.size() < b.size()) 
        return false;

    for (std::size_t i = 0; i < b.size(); i++) {
      if (::tolower(a[i]) != ::tolower(b[i]))
        return false;
    }

    return true;
}

void MultipartFormDataParser::appendBuf(const char* data, std::size_t n)
{
    auto remaining_size = unParseBufSize();
    // 将未解析的字符移到最前面
    if (remaining_size > 0 && cur_pos_ > 0) {
        for (std::size_t i = 0; i < remaining_size; i++) {
            buf_[i] = buf_[cur_pos_ + i];
        }
    }

    // 重置位置
    cur_pos_ = 0;
    end_pos_ = remaining_size;

    // 重新分配大小
    if (remaining_size + n > buf_.size()) {
        buf_.resize(remaining_size + n);
    }

    for (std::size_t i = 0; i < n; i++) {
        buf_[end_pos_ + i] = data[i];
    }

    // 设置末尾位置
    end_pos_ += n;
}

void MultipartFormDataParser::moveCurPos(std::size_t size)
{
    cur_pos_ += size;
}

string MultipartFormDataParser::bufHead(std::size_t len) const
{
    return buf_.substr(cur_pos_, len);
}

std::size_t MultipartFormDataParser::bufFind(const string& s) const
{
    auto c = s.front();

    std::size_t off = cur_pos_;
    while (off < end_pos_) {
        auto pos = off;
        // 查找与c相等的字符位置
        while (true) {
            if (pos == end_pos_) { return unParseBufSize(); }
            if (buf_[pos] == c) { break; }
            pos++;
        }

        // 剩余字符数量与s不匹配
        auto remaining_size = end_pos_ - pos;
        if (s.size() > remaining_size)
            return unParseBufSize();

        // 比较字符串
        if (compareSubStr(buf_, pos, end_pos_, s))
            return pos - cur_pos_;

        // 匹配失败，继续查找
        off = pos + 1;
    }

    return unParseBufSize();
}

bool MultipartFormDataParser::compareSubStr(const std::string& a,
    std::size_t cur,  std::size_t end, const std::string& b) const
{
    if (end - cur < b.size())
        return false;

    // 逐个字符比较
    for (std::size_t i = 0; i < b.size(); i++) {
        if (a[i + cur] != b[i]) 
            return false;
    }

    return true;
}

bool MultipartFormDataParser::compareBufSubStr(const string& s) const
{
    return compareSubStr(buf_, cur_pos_, end_pos_, s);
}

} // namespace https_server