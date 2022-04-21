#include "server.hpp"
#include "connection.hpp"
#include "request_handler.hpp"

#include <memory>
#include <fmt/format.h>

using std::string;
using asio::ip::address_v4;
using asio::ssl::context;
using asio::ip::tcp;
using asio::ssl::stream;
using std::error_code;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
using asio::redirect_error;

namespace https_server {

Server::Server(const string& address, const string& port,
    std::size_t io_context_pool_size,
    const Option& opt)
    : address_(address),
      port_(port),
      io_context_pool_(io_context_pool_size),
      ssl_context_(asio::ssl::context::sslv23),
      signals_(io_context_pool_.get_acceptor_singals_io_context()),
      acceptor_(io_context_pool_.get_acceptor_singals_io_context()),
      opt_(opt),
      req_handler_(service_maps_, opt),
      new_connection_() {

    ssl_context_.set_options(
    context::default_workarounds | 
    context::no_sslv2 );

    // 加载证书
    ssl_context_.use_certificate_chain_file(opt_.crtFilePath());
    // 加载私钥
    ssl_context_.use_private_key_file(opt_.privateKeyFilePath(), asio::ssl::context::pem);
    // 返回私钥密码
    ssl_context_.set_password_callback(
        [this](std::size_t, context::password_purpose) {
            return opt_.privateKeyPwd();
        }
    );

    // 注册程序终止的信号
    signals_.add(SIGINT);
    signals_.add(SIGTERM);
#if defined(SIGQUIT)
    signals_.add(SIGQUIT);
#endif

    // 启动一个协程处理信号的响应
    co_spawn(signals_.get_executor(), doAwaitStop(), detached);

    // 绑定地址和端口号
    tcp::resolver resolver(acceptor_.get_executor());
    tcp::endpoint endpoint = *resolver.resolve(address, port).begin();
    acceptor_.open(endpoint.protocol());
    // 打开地址复用选项
    acceptor_.set_option(tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    // 启动一个协程接受端口的网络请求
    co_spawn(acceptor_.get_executor(), doAccept(), detached);
}

void Server::addService(const std::string& path, Service& service) {
    service_maps_.emplace(path, service);
}

void Server::run() {
    fmt::print("Server is running...\n");
    fmt::print("The link is like https://{}:{}\n", address_, port_);

    io_context_pool_.run();
}

awaitable<void> Server::doAwaitStop() {
    // 停止所有的异步操作以关闭服务器
    co_await signals_.async_wait(use_awaitable);
    io_context_pool_.stop();
    fmt::print("Bye...\n");
}

awaitable<void> Server::doAccept() {
    // 持续监听端口
    for (;;) {
        // 重新生成一个新连接
        new_connection_.reset(new Connection(
            io_context_pool_.get_io_context(),
            ssl_context_, req_handler_, 
            opt_));
        
        error_code ec;
        co_await acceptor_.async_accept(new_connection_->socket(),
                                    redirect_error(use_awaitable, ec));
        // 启动一个连接
        if (!ec)
            new_connection_->start();
    }
}

} // namespace https_server