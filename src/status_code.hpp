#pragma once

#include <string>

namespace https_server {

enum class StatusCode {
    ok = 200,
    created = 201,
    accepted = 202,
    no_content = 204,
    partial_content = 206,
    multiple_choices = 300,
    moved_permanently = 301,
    moved_temporarily = 302,
    not_modified = 304,
    bad_request = 400,
    unauthorized = 401,
    forbidden = 403,
    not_found = 404,
    request_timeout = 408,
    length_required = 411,
    payload_too_large = 413,
    uri_too_long = 414,
    range_not_satisfiable = 416,
    internal_server_error = 500,
    not_implemented = 501,
    bad_gateway = 502,
    service_unavailable = 503,
    http_version_not_supported = 505
};

namespace status_code {

// 200 -> HTTP/1.1 200 OK\r\n
const std::string& statusToResponseHeader(const StatusCode& code);

// 404 -> <html>
//        <head><style>h1 {text-align: center;}</style><title>Not Found</title></head>
//        <body><h1>404 Not Found</h1></body>
//        </html>
const std::string& statusToResponseBody(const StatusCode& code);

} // namespace status_code

} // namespace https_server