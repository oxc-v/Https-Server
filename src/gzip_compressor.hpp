#pragma once

#include "compressor.hpp"
#include <zlib.h>

namespace https_server {

class GzipCompressor : public Compressor 
{

public:
    GzipCompressor();
    ~GzipCompressor();

    virtual bool compress(const char* d, std::size_t l, 
            bool last, Callback callback) override;

private:
    bool is_valid_ = false;
    z_stream strm_;
};

} // namespace https_server