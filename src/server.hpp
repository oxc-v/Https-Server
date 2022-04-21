#pragma once

#include "service.hpp"
#include "request_handler.hpp"
#include "io_context_pool.hpp"
#include "option.hpp"
#include "connection.hpp"

#include <asio.hpp>
#include <asio/ssl.hpp>

#include <map>
#include <vector>
#include <string>

namespace https_server {

class Server {
public:
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    explicit Server(const std::string& address, const std::string& port,
        std::size_t io_context_pool_size = 8,
        const Option& opt = Option());

    // 添加对应的路径和服务
    void addService(const std::string& path, Service& service);

    // 执行io_context循环
    void run();

private:
    // 服务器的配置选项
    Option opt_;

    std::map<const std::string, Service&> service_maps_;

    asio::ssl::context ssl_context_;

    // 用于执行异步操作的io_context对象池，默认为8个
    IoContextPool io_context_pool_;

    // 用于注册进程终止的通知
    asio::signal_set signals_;

    // 监听连接
    asio::ip::tcp::acceptor acceptor_;

    // 请求处理器
    RequestHandler req_handler_;

    // 地址
    const std::string address_;

    // 端口号
    const std::string port_;

    // 新的连接
    connection_ptr new_connection_;

    // 执行异步监听操作
    asio::awaitable<void> doAccept();

    // 等待停止服务器的请求
    asio::awaitable<void> doAwaitStop();
};

} // namespace https_server