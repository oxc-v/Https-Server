#include "request_handler.hpp"

#include "request.hpp"
#include "service.hpp"
#include "connection.hpp"
#include "brotli_compressor.hpp"
#include "gzip_compressor.hpp"
#include "no_compressor.hpp"

#include <fmt/format.h>

#include <random>

using std::string;
using std::unique_ptr;
using std::make_unique;

namespace https_server {

RequestHandler::RequestHandler(
            std::map<const std::string, Service&>& service_maps,
            const Option& opt)
    : service_maps_(service_maps),
      opt_(opt) {}

void RequestHandler::handleRequest(Connection& conn, 
                const Request& req, Response& res) 
{
    // 匹配服务
    for (auto& service_map: service_maps_) {
        if (req.path == service_map.first) {
            // 将request和response交由service自行处理
            service_map.second.handleRequest(req, res);
            writeResponse(conn, req, res);
            return;
        }
    }

    // 找不到对应方法
    writeStockResponseWithStatus(conn, StatusCode::not_found);
}

string RequestHandler::makeMultipartDataBoundary()
{
    static const char data[] =
        "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device seed_gen;

    // Request 128 bits of entropy for initialization
    std::seed_seq seed_sequence{seed_gen(), seed_gen(), seed_gen(), seed_gen()};
    std::mt19937 engine(seed_sequence);

    std::string result = "--cpp-httpserver-multipart-data-";

    for (auto i = 0; i < 16; i++) {
        result += data[engine() % (sizeof(data) - 1)];
    }

    return result;
}

std::pair<size_t, size_t> RequestHandler::getRangeOffsetAndLength(
        const Request &req, std::size_t content_len, std::size_t index)
{
    auto r = req.ranges[index];

    if (r.first == -1 && r.second == -1) {
        return std::make_pair(0, content_len);
    }

    auto slen = static_cast<ssize_t>(content_len);

    if (r.first == -1) {
        r.first = (std::max)(static_cast<ssize_t>(0), slen - r.second);
        r.second = slen - 1;
    }

    if (r.second == -1) { r.second = slen - 1; }
    return std::make_pair(r.first, static_cast<size_t>(r.second - r.first) + 1);
}

string RequestHandler::makeContentRangeHeaderField(
    std::size_t offset, std::size_t len, std::size_t content_len)
{
    string field = "bytes ";
    field += std::to_string(offset);
    field += "-";
    field += std::to_string(offset + len - 1);
    field += "/";
    field += std::to_string(content_len);
    return field;
}

template <typename Token, typename Content>
bool RequestHandler::processMultipartRangesData(const Request& req, Response& res,
                                const std::string& boundary,
                                const std::string& content_type,
                                const std::size_t content_len,
                                Token token, Content content)
{
    for (std::size_t i = 0; i < req.ranges.size(); i++) {
        token("--");
        token(boundary);
        token("\r\n");
        if (!content_type.empty()) {
            token("Content-Type: ");
            token(content_type);
            token("\r\n");
        }

        auto offsets = getRangeOffsetAndLength(req, content_len, i);
        auto offset = offsets.first;
        auto length = offsets.second;

        token("Content-Range: ");
        token(makeContentRangeHeaderField(offset, length, content_len));
        token("\r\n");
        token("\r\n");
        if (!content(offset, length)) { return false; }
        token("\r\n");
    }

    token("--");
    token(boundary);
    token("--\r\n");

    return true;
}

ssize_t RequestHandler::getMultipartRangesDataLength(
                                const Request &req, 
                                Response &res,
                                const std::string &boundary,
                                const std::string &content_type,
                                const std::size_t content_len)
{
    ssize_t data_length = 0;

    if (!processMultipartRangesData(
        req, res, boundary, content_type, content_len,
        [&](const std::string &token) { data_length += token.size(); },
        [&](std::size_t offset, std::size_t length) {
            if (offset < content_len && length <= content_len) {
                data_length += length;
                return true;
            }
            return false;
        })) 
    {
        return -1;
    }

    return data_length;
}

bool RequestHandler::makeMultipartRangesData(
                const Request& req, Response& res,
                const std::string& boundary,
                const std::string& content_type,
                std::string& data)
{
    return processMultipartRangesData(
        req, res, boundary, content_type, res.body.size(),
        [&](const std::string &token) { data += token; },
        [&](size_t offset, size_t length) {
            if (offset < res.body.size() && length <= res.body.size()) {
                data += res.body.substr(offset, length);
                return true;
            }
            return false;
        });
}

void RequestHandler::writeMultipartRangesData(Connection& conn, 
                        const Request& req, Response& res,
                        const std::string& boundary,
                        const std::string& content_type)
{
    processMultipartRangesData(
        req, res, boundary, content_type, res.content_len_,
        [&](const string& token) { conn.doWrite(token.c_str(), token.size()); },
        [&](size_t offset, size_t length) {
            return writeContent(conn, res.content_provider_, offset, length);
        });
}

void RequestHandler::writeResponse(Connection& conn, 
            const Request& req, Response& res)
{
    if (req.ranges.empty()) {
        res.status = StatusCode::ok;
    } else {
        res.status = StatusCode::partial_content;
    }

    string boundary;
    string content_type;
    if (req.ranges.size() > 1) {
        boundary = makeMultipartDataBoundary();
        content_type = res.getHeaderValue("Content-Type");
        res.setHeader("Content-Type",
                    "multipart/byteranges; boundary=" + boundary);
    }

    auto encoding_type = encoding_type::encodingType(req, res, opt_);

    if (res.body.empty()) {
        if (res.content_provider_) {
            std::size_t length = 0;
            if (req.ranges.empty()) {
                length = res.content_len_;
            } else if (req.ranges.size() == 1) {
                auto offsets =
                    getRangeOffsetAndLength(req, res.content_len_, 0);
                auto offset = offsets.first;
                length = offsets.second;
                auto content_range = makeContentRangeHeaderField(
                    offset, length, res.content_len_);
                res.setHeader("Content-Range", content_range);
                if (offset >= res.content_len_ || length > res.content_len_) {
                    length = 0;
                    res.status = StatusCode::range_not_satisfiable;
                }
            } else {
                auto len = getMultipartRangesDataLength(
                    req, res, boundary, content_type, res.content_len_);
                if (len == -1) {
                    res.status = StatusCode::range_not_satisfiable;
                    length = 0;
                } else {
                    length = static_cast<std::size_t>(len);
                }
            }
            res.setHeader("Content-Length", std::to_string(length));
        } else if (res.content_provider_without_length_) {
            res.setHeader("Transfer-Encoding", "chunked");
            if (encoding_type == EncodingType::Brotli) {
                res.setHeader("Content-Encoding", "br");
            } else if (encoding_type == EncodingType::Gzip) {
                res.setHeader("Content-Encoding", "gzip");
            }
        }
    } else {
        if (req.ranges.empty()) {
            ;
        } else if (req.ranges.size() == 1) {
            auto offsets =
                getRangeOffsetAndLength(req, res.body.size(), 0);
            auto offset = offsets.first;
            auto length = offsets.second;
            auto content_range = makeContentRangeHeaderField(
                offset, length, res.body.size());
            res.setHeader("Content-Range", content_range);
            if (offset < res.body.size() && length <= res.body.size()) {
                res.body = res.body.substr(offset, length);
            } else {
                res.body.clear();
                res.status = StatusCode::range_not_satisfiable;
            }
        } else {
            std::string data;
            if (makeMultipartRangesData(req, res, 
                        boundary, content_type, data)) {
                res.body.swap(data);
            } else {
                res.body.clear();
                res.status = StatusCode::range_not_satisfiable;
            }
        }

        // 压缩数据
        if (encoding_type != EncodingType::None) {
            unique_ptr<Compressor> compressor;
            string content_encoding;

            if (encoding_type == EncodingType::Gzip) {
                compressor = make_unique<GzipCompressor>();
                content_encoding = "gzip";
            } else if (encoding_type == EncodingType::Brotli) {
                compressor = make_unique<BrotliCompressor>();
                content_encoding = "br";
            }

            if (compressor) {
                string compressed;
                if (compressor->compress(res.body.data(), res.body.size(), true,
                                [&](const char *data, size_t data_len) {
                                    compressed.append(data, data_len);
                                    return true;
                                })) {
                    res.body.swap(compressed);
                    res.setHeader("Content-Encoding", content_encoding);
                }
            }
        }

        auto length = std::to_string(res.body.size());
        res.setHeader("Content-Length", length);
    }

    if (req.getHeaderValue("Connection") == "close") {
        res.setHeader("Connection", "close");
    } else if (opt_.connectionTimeout() != 0) {
        res.setHeader("Keep-Alive", 
            fmt::format("timeout={}", opt_.connectionTimeout()));
    }

	if (!res.hasHeader("Content-Type") &&
		(!res.body.empty() || res.content_len_ > 0 
		|| res.content_provider_)) {
		res.setHeader("Content-Type", "text/plain");
	}

	if (!res.hasHeader("Content-Length") && 
		res.body.empty() && !res.content_len_ && 
		!res.content_provider_without_length_) {
    	res.setHeader("Content-Length", "0");
	}

	if (!res.hasHeader("Accept-Ranges") 
		&& req.method == "HEAD") {
    	res.setHeader("Accept-Ranges", "bytes");
	}

    if (res.status == StatusCode::range_not_satisfiable) {
        writeStockResponseWithStatus(conn, StatusCode::range_not_satisfiable);
        return;
    }

    writeHTTPStatus(conn, res.status);
    writeHeaders(conn, res);

    if (req.method != "HEAD") {
        if (!res.body.empty()) {
            writeContentWithoutProvider(conn, res);
        } else if (res.content_provider_ || 
                res.content_provider_without_length_) {
            writeContentWithProvider(conn, req, res, boundary, content_type);
        }
    }
}

bool RequestHandler::writeContent(Connection& conn, 
            const ContentProvider& content_provider,
            std::size_t offset, std::size_t length)
{
    std::size_t end_offset = offset + length;

	DataSink data_sink;

	data_sink.write = [&](const char* data, std::size_t len) {
        if (data_sink.is_writable) {
            data_sink.is_writable = conn.doWrite(data, len);
        }
	};

	content_provider(offset, end_offset - offset, data_sink);

    return data_sink.is_writable;
}

void RequestHandler::writeStockResponseWithStatus(
                    Connection& conn, const StatusCode& status)
{
    auto res = Response::stockResponse(status);
    writeHTTPStatus(conn, status);
    writeHeaders(conn, res);
    writeContentWithoutProvider(conn, res);
}

void RequestHandler::writeContentWithProvider(
                Connection& conn, const Request& req,
                Response& res, const std::string& boundary,
                const std::string& content_type)
{
    if (res.content_provider_) {
        if (req.ranges.empty()) {
            writeContent(conn, res.content_provider_, 
                    0, res.content_len_);
        } else if (req.ranges.size() == 1) {
            auto offsets =
                    getRangeOffsetAndLength(req, res.content_len_, 0);
            auto offset = offsets.first;
            auto length = offsets.second;
            writeContent(conn, res.content_provider_, offset, length);
        } else {
            writeMultipartRangesData(conn, req, res, boundary, content_type);
        }
    } else if (res.content_provider_without_length_) {
        auto type = encoding_type::encodingType(req, res, opt_);

        unique_ptr<Compressor> compressor;
        if (type == EncodingType::Gzip) {
            compressor = make_unique<GzipCompressor>();
        } else if (type == EncodingType::Brotli) {
            compressor = make_unique<BrotliCompressor>();
        } else {
            compressor = make_unique<NoCompressor>();
        }

        writeContentChunked(conn, 
                    res.content_provider_without_length_,
                    *compressor);
    }
}

void RequestHandler::writeContentChunked(Connection& conn, 
                const ContentProviderWithoutLength& provider,
                Compressor& compressor)
{
    DataSink data_sink;

    data_sink.write = [&](const char* d, std::size_t l) {
        if (data_sink.is_writable && l > 0) {
            string payload;
            if (compressor.compress(d, l, false,
                    [&](const char* data, std::size_t data_len) {
                        payload.append(data, data_len);
                        return true;
                    })) 
            {
                if (!payload.empty()) {
                    auto chunk =
                        fmt::format("{:x}", payload.size()) + "\r\n" + payload + "\r\n";
                    conn.doWrite(chunk.c_str(), chunk.size());
                }
            } else {
                data_sink.is_writable = false;
            }
        }
    };

    data_sink.done = [&](void) {
        if (!data_sink.is_writable) { return; }

        std::string payload;
        if (!compressor.compress(nullptr, 0, true,
                [&](const char *data, size_t data_len) {
                    payload.append(data, data_len);
                    return true;
                })) 
        {
            data_sink.is_writable = false;
            return;
        }

        if (!payload.empty()) {
            auto chunk =
                    fmt::format("{:x}", payload.size()) + "\r\n" + payload + "\r\n";
            conn.doWrite(chunk.c_str(), chunk.size());
        }

        static const string done_marker("0\r\n\r\n");
        conn.doWrite(done_marker.c_str(), done_marker.size());
    };

    provider(data_sink);
}

void RequestHandler::writeHeaders(Connection& conn, Response& res)
{
    string data;
    // 头部信息
    for (auto& h: res.headers) {
        data += h.name;
        data += string(name_value_separator_, 2);
        data += h.value;
        data += string(crlf_, 2);
    }

    // 空行
    data += string(crlf_, 2);

    conn.doWrite(data.c_str(), data.size());
}

void RequestHandler::writeHTTPStatus(Connection& conn, const StatusCode& status)
{
    auto data = status_code::statusToResponseHeader(status);

    conn.doWrite(data.c_str(), data.size());
}

void RequestHandler::writeContentWithoutProvider(
        Connection& conn, Response& res)
{
    conn.doWrite(res.body.c_str(), res.body.size());
}

} // namespace https_server