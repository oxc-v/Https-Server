#pragma once

#include <functional>
#include <string>

class DataSink
{
public:
    DataSink(const DataSink &) = delete;
    DataSink &operator=(const DataSink &) = delete;
    DataSink(DataSink &&) = delete;
    DataSink &operator=(DataSink &&) = delete;

    DataSink() = default;

    bool is_writable = true;

    std::function<void(const char* data, std::size_t len)> write;

    std::function<void()> done;
};