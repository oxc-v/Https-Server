#pragma once

#include "compressor.hpp"

namespace https_server {

class NoCompressor : public Compressor
{
public:
    virtual ~NoCompressor() = default;

    virtual bool compress(const char* data, 
                std::size_t data_length, bool /*last*/,
                Callback callback) override;
};

} // namespace https_server