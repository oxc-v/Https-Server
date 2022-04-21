#include "gzip_compressor.hpp"

#include <cstring>
#include <cassert>
#include <limits>

namespace https_server {

GzipCompressor::GzipCompressor() 
{
    std::memset(&strm_, 0, sizeof(strm_));
    strm_.zalloc = Z_NULL;
    strm_.zfree = Z_NULL;
    strm_.opaque = Z_NULL;

    is_valid_ = deflateInit2(&strm_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 
            31, 8, Z_DEFAULT_STRATEGY) == Z_OK;
}

GzipCompressor::~GzipCompressor() 
{ 
    deflateEnd(&strm_); 
}

bool GzipCompressor::compress(const char* d, std::size_t l,
                        bool last, Callback callback) 
{
    assert(is_valid_);

    do {
        constexpr std::size_t max_avail_in =
            (std::numeric_limits<decltype(strm_.avail_in)>::max)();

        strm_.avail_in = static_cast<decltype(strm_.avail_in)>(
            (std::min)(l, max_avail_in));
        strm_.next_in = const_cast<Bytef *>(reinterpret_cast<const Bytef *>(d));

        l -= strm_.avail_in;
        d += strm_.avail_in;

        auto flush = (last && l == 0) ? Z_FINISH : Z_NO_FLUSH;
        int ret = Z_OK;

        std::array<char, 8192u> buff{};
        do {
            strm_.avail_out = static_cast<uInt>(buff.size());
            strm_.next_out = reinterpret_cast<Bytef *>(buff.data());

            ret = deflate(&strm_, flush);
            if (ret == Z_STREAM_ERROR) { return false; }

            if (!callback(buff.data(), buff.size() - strm_.avail_out)) {
                return false;
            }
        } while (strm_.avail_out == 0);

        assert((flush == Z_FINISH && ret == Z_STREAM_END) ||
                (flush == Z_NO_FLUSH && ret == Z_OK));
        assert(strm_.avail_in == 0);
    } while (l > 0);

    return true;
}

} // namespace https_server