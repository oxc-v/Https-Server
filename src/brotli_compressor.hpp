#pragma once

#include "compressor.hpp"
#include <brotli/encode.h>

namespace https_server {

// brotli压缩器
class BrotliCompressor : public Compressor
{
public:
    BrotliCompressor(const BrotliCompressor &) = delete;
    BrotliCompressor& operator=(const BrotliCompressor&) = delete;

    BrotliCompressor();
    ~BrotliCompressor();

    virtual bool compress(const char *d, std::size_t l, 
                    bool last, Callback callback) override;

private:
    BrotliEncoderState* state_ = nullptr;
};

} // namespace https_server