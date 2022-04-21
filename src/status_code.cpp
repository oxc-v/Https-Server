#include "status_code.hpp"



using std::string;



namespace https_server {
namespace status_code {

struct StatusMapping
{
    const StatusCode code;
    const string header_str;
    const string body_str;
} status_mappings[] =
{
    {StatusCode::ok, "HTTP/1.1 200 OK\r\n",
        ""
    },
    {StatusCode::created, "HTTP/1.1 201 Created\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Created</title>"\
        "</head>"\
        "<body><h1>201 Created</h1></body>"\
        "</html>"
    },
    {StatusCode::accepted, "HTTP/1.1 202 Accepted\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Accepted</title></head>"\
        "<body><h1>202 Accepted</h1></body>"\
        "</html>"
    },
    {StatusCode::no_content, "HTTP/1.1 204 No Content\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>No Content</title>"\
        "</head>"\
        "<body><h1>204 No Content</h1></body>"\
        "</html>"
    },
    {StatusCode::partial_content,  "HTTP/1.1 206 Partial Content\r\n",
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Partial Content</title>"\
        "</head>"\
        "<body><h1>206 Partial Content</h1></body>"\
        "</html>"
    },
    {StatusCode::multiple_choices, "HTTP/1.1 300 Multiple Choices\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Multiple Choices</title>"\
        "</head>"\
        "<body><h1>300 Multiple Choices</h1></body>"\
        "</html>"
    },
    {StatusCode::moved_permanently, "HTTP/1.1 301 Moved Permanently\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Moved Permanently</title>"\
        "</head>"\
        "<body><h1>301 Moved Permanently</h1></body>"\
        "</html>"
    },
    {StatusCode::moved_temporarily, "HTTP/1.1 302 Moved Temporarily\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Moved Temporarily</title>"\
        "</head>"\
        "<body><h1>302 Moved Temporarily</h1></body>"\
        "</html>"
    },
    {StatusCode::not_modified, "HTTP/1.1 304 Not Modified\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Not Modified</title>"\
        "</head>"\
        "<body><h1>304 Not Modified</h1></body>"\
        "</html>"
    },
    {StatusCode::bad_request, "HTTP/1.1 400 Bad Request\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Bad Request</title>"\
        "</head>"\
        "<body><h1>400 Bad Request</h1></body>"\
        "</html>"
    },
    {StatusCode::unauthorized, "HTTP/1.1 401 Unauthorized\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Unauthorized</title>"\
        "</head>"\
        "<body><h1>401 Unauthorized</h1></body>"\
        "</html>"
    },
    {StatusCode::forbidden, "HTTP/1.1 403 Forbidden\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Forbidden</title>"\
        "</head>"\
        "<body><h1>403 Forbidden</h1></body>"\
        "</html>"
    },
    {StatusCode::not_found, "HTTP/1.1 404 Not Found\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Not Found</title>"\
        "</head>"\
        "<body><h1>404 Not Found</h1></body>"\
        "</html>"
    },
    {StatusCode::request_timeout, "HTTP/1.1 408 Request Timeout\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Request Timeout</title>"\
        "</head>"\
        "<body><h1>408 Request Timeout</h1></body>"\
        "</html>"
    },
    {StatusCode::length_required, "HTTP/1.1 411 Length Required\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Length Required</title>"\
        "</head>"\
        "<body><h1>411 Length Required</h1></body>"\
        "</html>"
    },
    {StatusCode::payload_too_large, "HTTP/1.1 413 Payload Too Large\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Payload Too Large</title>"\
        "</head>"\
        "<body><h1>413 Payload Too Large</h1></body>"\
        "</html>"
    },
    {StatusCode::uri_too_long, "HTTP/1.1 414 URI Too Long\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>URI Too Long</title>"\
        "</head>"\
        "<body><h1>414 URI Too Long</h1></body>"\
        "</html>"
    },
    {StatusCode::range_not_satisfiable, "HTTP/1.1 416 Range Not Satisfiable\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Range Not Satisfiable</title>"\
        "</head>"\
        "<body><h1>416 Range Not Satisfiable</h1></body>"\
        "</html>"
    },
    {StatusCode::internal_server_error, "HTTP/1.1 500 Internal Server Error\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Internal Server Error</title>"\
        "</head>"\
        "<body><h1>500 Internal Server Error</h1></body>"\
        "</html>"
    },
    {StatusCode::not_implemented, "HTTP/1.1 501 Not Implemented\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><titleNot Implemented</title>"\
        "</head>"\
        "<body><h1>501 Not Implemented</h1></body>"\
        "</html>"
    },
    {StatusCode::bad_gateway, "HTTP/1.1 502 Bad Gateway\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Bad Gateway</title>"\
        "</head>"\
        "<body><h1>502 Bad Gateway</h1></body>"\
        "</html>"
    },
    {StatusCode::service_unavailable, "HTTP/1.1 503 Service Unavailable\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Service Unavailable</title>"\
        "</head>"\
        "<body><h1>503 Service Unavailable</h1></body>"\
        "</html>"
    },
    {StatusCode::http_version_not_supported, "HTTP/1.1 505 HTTP Version not supported\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>HTTP Version not supported</title>"\
        "</head>"\
        "<body><h1>505 HTTP Version not supported</h1></body>"\
        "</html>"
    }
};

const StatusMapping defualt{
    StatusCode::internal_server_error, "HTTP/1.1 500 Internal Server Error\r\n", 
        "<html>"\
        "<head><style>h1 {text-align: center;}</style><title>Internal Server Error</title>"\
        "</head>"\
        "<body><h1>500 Internal Server Error</h1></body>"\
        "</html>"
};

const StatusMapping& statusToMapping(const StatusCode& code)
{
    for (const auto& mapping: status_mappings) {
        if (mapping.code == code)
            return mapping;
    }

    return defualt;
}

const string& statusToResponseHeader(const StatusCode& code)
{
    const auto& status_mapping = statusToMapping(code);
    return status_mapping.header_str;
}

const string& statusToResponseBody(const StatusCode& code)
{
    const auto& status_mapping = statusToMapping(code);
    return status_mapping.body_str;
}

} // namespace status_code
} // namespace https_server