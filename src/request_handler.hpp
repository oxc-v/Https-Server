#pragma once

#include "response.hpp"
#include "option.hpp"
#include "compressor.hpp"

#include <vector>
#include <string>
#include <map>

namespace https_server {

class Request;
class Service;
class Connection;

// 所有请求的通用处理器
class RequestHandler {
public:
    RequestHandler(const RequestHandler&) = delete;
    RequestHandler& operator=(const RequestHandler&) = delete;

    explicit RequestHandler(
                std::map<const std::string, Service&>& service_maps,
                const Option& opt);

    // 处理请求并生成响应信息
    void handleRequest(Connection& conn, const Request& req, Response& res);

    // 根据状态码发送响应的固定响应
    void writeStockResponseWithStatus(Connection& conn, 
                            const StatusCode& status);

private:
    // 服务映射
    std::map<const std::string, Service&>& service_maps_;

    const char name_value_separator_[2] = {':', ' '};

    const char crlf_[2] = {'\r', '\n'};

    // 配置
    const Option& opt_;

    // 生成一个随机分界线
    std::string makeMultipartDataBoundary();

    // 根据range获取数据的偏移量和长度
    // content_len: 数据总长度
    // index: ranges索引
    // first: 偏移量  second: 长度
    std::pair<size_t, size_t> getRangeOffsetAndLength(const Request &req, 
        std::size_t content_len, std::size_t index);

    // 获取多范围的内容长度
    // boundary: 分界线
    // content_type: 数据类型
    // content_len: 数据长度
    // 返回值：-1（请求范围不合法） 真实长度
    ssize_t getMultipartRangesDataLength(const Request& req, 
                                Response& res,
                                const std::string& boundary,
                                const std::string& content_type,
                                const std::size_t content_len);
    
    // 返回一个范围片段
    // 如bytes 0-10/100
    // offset: 数据偏移量
    // len: 数据长度
    // content_len: 总长度
    std::string makeContentRangeHeaderField(std::size_t offset, std::size_t len,
                                    std::size_t content_len);
    
    // 处理范围请求
    // Token: 处理分界线
    // Content: 处理数据内容
    template <typename Token, typename Content>
    bool processMultipartRangesData(const Request& req, Response& res,
                                const std::string& boundary,
                                const std::string& content_type,
                                const std::size_t content_len,
                                Token stoken, Content content);

    // 使用content_provider拼接多重范围数据
    void writeMultipartRangesData(Connection& conn, 
                        const Request& req, Response& res,
                        const std::string& boundary,
                        const std::string& content_type);

    // 根据req body内容拼接多重范围数据
    // data: 拼接后的数据
    bool makeMultipartRangesData(const Request& req, Response& res,
                                const std::string& boundary,
                                const std::string& content_type,
                                std::string& data);

    // 向客户端发送响应
    void writeResponse(Connection& conn, 
                    const Request& req, Response& res);

    // 根据provider处理一个range
    // false 写操作发生错误;
    // true 写操作完成
    bool writeContent(Connection& conn, 
                    const ContentProvider& content_provider,
                    std::size_t offset, std::size_t length);
    
    // 根据provider将数据发送到客户端
    void writeContentWithProvider(Connection& conn, const Request& req,
                            Response& res, const std::string& boundary,
                            const std::string& content_type);
    
    void writeContentChunked(Connection& conn, 
                const ContentProviderWithoutLength& provider,
                Compressor& compressor); 
    
    // 将头部信息发送到客户端
    void writeHeaders(Connection& conn, Response& res);

    // 将状态信息发送到客户端
    void writeHTTPStatus(Connection& conn, const StatusCode& status);

    // 使用body发送响应
    void writeContentWithoutProvider(Connection& conn, Response& res);
};

} // namespace https_server