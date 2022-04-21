#pragma once

#include "request.hpp"
#include "response.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include "option.hpp"

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <memory>
#include <array>


namespace https_server {

// 设置套接字别名
using ssl_socket = asio::ssl::stream<asio::ip::tcp::socket>;

class Connection : public std::enable_shared_from_this<Connection> {
private:
    // 请求
    Request req_;

    // 响应
    Response res_;

    // 解析请求
    RequestParser req_parser_;

    // 处理响应
    RequestHandler& req_handler_;

    // ssl套接字
    ssl_socket socket_;

    std::array<char, 8192> buffer_;

    // 设置定时器，超时关闭连接
    asio::steady_timer timer_;

    // 配置信息
    const Option& opt_;

    // 异步等待定时器到期，并执行相关操作
    asio::awaitable<void> doDeadline(asio::steady_timer& timer);

    // tls握手
    asio::awaitable<void> doHandshake();

    // 异步的读操作
    asio::awaitable<void> doRead();

    // 重置本次连接
    // 如果客户端需要保持长连接，那么需要在下次读数据时
    // 重置req，res，req_parser等对象
    void reset();

public:
    // 禁用赋值和复制
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(asio::io_context& io_context,
        asio::ssl::context& context,
        RequestHandler& handler,
        const Option& opt);
    
    // 测试
    // ~Connection() { std::cout << "connection close..." << std::endl; }

    // 开始本次连接的第一个异步操作
    void start();

    // 停止本次连接的所有异步操作
    void stop();

    // 将数据写入socket中
    // 注意：该操作是阻塞的
    // 当发生错误时返回false, 反之返回true
    bool doWrite(const char* data, std::size_t len);

    // 返回当前连接的套接字引用
    ssl_socket::lowest_layer_type& socket();
};

using connection_ptr = std::shared_ptr<Connection>;

} // namespace https_server