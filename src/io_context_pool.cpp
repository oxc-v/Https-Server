#include "io_context_pool.hpp"

#include <thread>

using asio::make_work_guard;

namespace https_server {

IoContextPool::IoContextPool(std::size_t pool_size)
    : next_io_context_(1)
{
    if (pool_size < 2)
        throw std::runtime_error("io_context_pool size less than 2");

    // 创建pool_size数量的io_context对象
    // io_context在没有任何异步操作时将退出事件循环，
    // 通过创建跟踪针对io_context工作的执行器，可以保持调用的运行io_context.run()
    for (std::size_t i = 0; i < pool_size; ++i) {
        auto io_context = std::make_shared<asio::io_context>();
        io_contexts_.push_back(io_context);
        work_.push_back(make_work_guard(*io_context));
    }
}

void IoContextPool::run()
{
    // 创建pool_size数量的线程运行io_contexts
    // 即一个线程对应一个io_context
    std::vector<std::shared_ptr<std::thread>> threads;
    for (std::size_t i = 0; i < io_contexts_.size(); ++i) {
        auto cotx = io_contexts_[i];
        auto thread = std::make_shared<std::thread>(
            [cotx]() {
                cotx->run();
            }
        );
        threads.push_back(thread);
    }


    // 阻塞等待所有线程退出
    for (auto& thread: threads)
        thread->join();
}

void IoContextPool::stop()
{
    // 显式地停用io_cotext
    for (auto& io_context: io_contexts_)
        io_context->stop();
}

asio::io_context& IoContextPool::get_io_context()
{
    // 采用轮询的方式决定下一个要使用的io_context
    asio::io_context& io_context = *io_contexts_[next_io_context_];
    ++next_io_context_;
    if (next_io_context_ == io_contexts_.size())
        next_io_context_ = 1;
    return io_context;
}

asio::io_context& IoContextPool::get_acceptor_singals_io_context()
{
    // 第一个io_context分配给特定的角色
    asio::io_context& io_context = *io_contexts_[0];
    return io_context;
}

} // namespace https_server