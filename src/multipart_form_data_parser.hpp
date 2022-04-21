#pragma once

#include "multipart_form_data.hpp"
#include "result_type.hpp"

#include <string>

namespace https_server {

class Request;

class MultipartFormDataParser 
{
public:
    MultipartFormDataParser();

    // 设置边界线
    void setBoundary(std::string&& boundary);

    // 解析form-data
    // buf是数据的开始位置，n为数据的总长度
    ResultType parse(Request& req, 
                        const char* buf, std::size_t n);
    
    // 重置解析状态
    void reset();

private:
    // 文件信息
    MultipartFormData file_;

    // 边界线
    std::string boundary_;

    // 存放数据
    std::string buf_;

    // 当前解析位置
    std::size_t cur_pos_;

    // 0 + buf_.size()
    std::size_t end_pos_;

    // 返回剩余未解析的数据大小
    size_t unParseBufSize() const;

    // 清除文件信息
    void clearFileInfo();

    // 比较两个字符串，忽略大小写
    // 不全等比较
    bool compareWithCaseIgnore(const std::string& a,
                        const std::string& b) const;

    // 移动当前解析位置，cur_pos_ += size
    void moveCurPos(std::size_t size);

    // 往buf_中添加数据
    void appendBuf(const char *data, std::size_t n);

    // 截取buf_中 [cur, cur + len) 之间的字符串
    std::string bufHead(std::size_t len) const;

    // 从buf_的cur_pos_位置开始查找与s相等的字符串
    // 返回值为 pos - cur_pos_ ，其中pos为s的第一个字符
    // 对应的buf_中的位置
    // 如果返回值等于当前unParseBufSize()，代表找不到与s匹配的字符串
    std::size_t bufFind(const std::string& s) const;

    // 从a字符串的cur位置开始，逐个字符与b进行比较
    // 如果比较的字符全部相等，则返回true，反之返回false
    // cur: a中的某个位置
    // end: 0 + a.size()
    bool compareSubStr(const std::string& a, std::size_t cur, std::size_t end,
                const std::string& b) const;

    // 调用 compareSubStr(buf_, cur_pos_, end_pos_, s);
    bool compareBufSubStr(const std::string& s) const;

    // 当前解析状态
    enum ParseState 
    {
        initial_boundary,
        new_entry,
        headers,
        body,
        boundary
    } state_;
};

} // namespace https_server