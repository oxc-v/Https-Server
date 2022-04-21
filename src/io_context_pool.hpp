#pragma once

#include <asio.hpp>

#include <vector>
#include <memory>
#include <list>

namespace https_server {

// io_context对象池
class IoContextPool
{
public:
    IoContextPool(const IoContextPool&) = delete;
    IoContextPool& operator=(const IoContextPool&) = delete;

    explicit IoContextPool(std::size_t pool_size);

    // 执行pool中的所有io_context对象
    void run();

    // 停止pool中的所有io_context对象
    void stop();

    // 从pool中获取一个io_context对象使用
    asio::io_context& get_io_context();

    // 从pool获取一个专用的io_context
    asio::io_context& get_acceptor_singals_io_context();

private:
    using io_context_ptr = std::shared_ptr<asio::io_context>;
    using io_context_work = asio::executor_work_guard<
        asio::io_context::executor_type>;

    std::vector<io_context_ptr> io_contexts_;

    std::list<io_context_work> work_;

    // 该索引指向下一个连接将要使用的io_context对象的vector索引
    std::size_t next_io_context_;
};

} // namespace https_server