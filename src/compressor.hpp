#pragma once

#include <cstddef>
#include <functional>

namespace https_server {

class Compressor
{
public:
    virtual ~Compressor() = default;

    using Callback = std::function<bool(const char* d, std::size_t l)>;
    virtual bool compress(const char* d, std::size_t l, 
                    bool last, Callback callback) = 0;
};

} // namespace https_server