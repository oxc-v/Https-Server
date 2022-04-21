#include "connection.hpp"
#include "result_type.hpp"

#include <vector>

using asio::ip::tcp;
using std::vector;
using std::string;
using asio::buffer;
using asio::const_buffer;
using asio::ssl::stream_base;
using asio::steady_timer;
using std::error_code;
using asio::awaitable;
using asio::co_spawn;
using asio::detached;
using asio::use_awaitable;
using asio::redirect_error;

namespace https_server {

Connection::Connection(asio::io_context& io_context,
    asio::ssl::context& context,
    RequestHandler& handler,
	const Option& opt)
    : socket_(io_context, context),
	  timer_(socket_.get_executor()),
      req_handler_(handler),
	  req_parser_(RequestParser(opt)),
	  opt_(opt)
{
	timer_.expires_at(steady_timer::time_point::max());
}

void Connection::start() {
	if (opt_.connectionTimeout() != 0) {
		// 启动一个协程执行定时器到期操作
		co_spawn(timer_.get_executor(), 
			[this, self = shared_from_this()] { return self->doDeadline(timer_); },
			detached);

		timer_.expires_after(std::chrono::seconds(opt_.connectionTimeout()));
	}

    // 启动一个协程进行ssl握手操作
	co_spawn(socket_.get_executor(), 
		[self = shared_from_this()] { return self->doHandshake(); }, 
		detached);
}

void Connection::stop() {
	timer_.cancel();

	asio::error_code ignored_ec;
	socket_.lowest_layer().shutdown(tcp::socket::shutdown_both,
        		ignored_ec);
	socket_.lowest_layer().close();
}

ssl_socket::lowest_layer_type& Connection::socket()
{
	return socket_.lowest_layer();
}

void Connection::reset()
{
	req_ = Request();
	res_ = Response();
	req_parser_.reset();
}

awaitable<void> Connection::doDeadline(steady_timer& timer)
{
	while (true) {
		co_await timer.async_wait(use_awaitable);

		if (!socket_.lowest_layer().is_open())
			break;

		// 检查期限是否已过。
		// 这里比较最后期限，因为deadline会被移动
		if (timer.expiry() <= steady_timer::clock_type::now()) {
			// 关闭闲置连接
			stop();
			break;
		}
	}
}

awaitable<void> Connection::doHandshake() {
	error_code ec;
	co_await socket_.async_handshake(stream_base::server,
								redirect_error(use_awaitable, ec));
	if (!ec) {
		// 启动一个协程等待读取数据
		co_spawn(socket_.get_executor(), 
			[self = shared_from_this()] { return self->doRead(); }, 
			detached);
	} else if (ec != asio::error::operation_aborted) {
		// ssl握手失败，断开本次连接
		this->stop();
	}
}

awaitable<void> Connection::doRead() {
	while (true) {
		if (!socket_.lowest_layer().is_open())
			break;

		error_code ec;
		std::size_t n = co_await socket_.async_read_some(buffer(buffer_),
									redirect_error(use_awaitable, ec));

		if (!ec) {
			// 解析HTTP消息
			ResultType result;
			std::tie(result, std::ignore) = req_parser_.parse(
					req_, res_, buffer_.data(), buffer_.data() + n);
			// cout << string(buffer_.data(), buffer_.data() + n) << endl;
			if (result == good) {
				// HTTP消息符合规范，开始处理请求
				req_.remote_addr = socket_.lowest_layer().remote_endpoint().address().to_string();
    			req_handler_.handleRequest(*this, req_, res_);
				reset();
    		} else if (result == bad) {
				// HTTP消息解析失败
				req_handler_.writeStockResponseWithStatus(*this, res_.status);
				break;
    		}
		} else if (ec != asio::error::operation_aborted) {
			// 读取错误，断开连接并退出循环
			this->stop();
			break;
		}
	}
}

bool Connection::doWrite(const char* data, std::size_t len)
{
	error_code ec;
	asio::write(socket_, buffer(data, len), ec);

	if (!ec) {
		return true;
	} else if (ec != asio::error::operation_aborted) {
		stop();
	}

	return false;
}

} // namespace https_server