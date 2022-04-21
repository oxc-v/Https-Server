#include "brotli_compressor.hpp"

#include <array>

using std::array;

namespace https_server {

BrotliCompressor::BrotliCompressor() {
    state_ = BrotliEncoderCreateInstance(nullptr, nullptr, nullptr);
}

BrotliCompressor::~BrotliCompressor() {
    BrotliEncoderDestroyInstance(state_);
}

bool BrotliCompressor::compress(const char* data, std::size_t data_length,
                            bool last, Callback callback)
{
    array<uint8_t, 8192u> buffer{};

    auto operation = last ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;
    auto available_in = data_length;
    auto next_in = reinterpret_cast<const uint8_t*>(data);

    for (;;) {
        if (last) {
            if (BrotliEncoderIsFinished(state_)) { break; }
        } else {
            if (!available_in) { break; }
        }

        auto available_out = buffer.size();
        auto next_out = buffer.data();

        if (!BrotliEncoderCompressStream(state_, operation, &available_in, &next_in,
                                        &available_out, &next_out, nullptr)) {
            return false;
        }

        auto output_bytes = buffer.size() - available_out;
        if (output_bytes) {
            callback(reinterpret_cast<const char*>(buffer.data()), output_bytes);
        }
    }

    return true;
}

} // namespace https_server