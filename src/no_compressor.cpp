#include "no_compressor.hpp"

namespace https_server {

bool NoCompressor::compress(const char* data, std::size_t data_length,
                            bool /*last*/, Callback callback) 
{
    if (!data_length) { return true; }
    return callback(data, data_length);
}

} // namespace https_server