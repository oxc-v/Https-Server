#include "response.hpp"

#include <cassert>

using std::string;
using std::vector;

namespace https_server {

void Response::setHeader(const string& name, const string& value) {
    string lw_name;
    for (int i = 0; i < name.size(); ++i)
        lw_name.push_back(tolower(name[i]));

    for (auto& header: headers) {
        if (header.name == lw_name) {
            header.value = value;
            return;
        }
    }

    headers.push_back(Header(lw_name, value));
}

void Response::appendHeader(const std::string& name, 
                    const std::string& value)
{
    if (headers.empty() || !hasHeader(name))
        return;

    auto v = getHeaderValue(name);
    setHeader(name, v + "," + value);
}

string Response::getHeaderValue(const string& name) const
{
    if (headers.empty())
        return string();

    string lw_name;
    for (int i = 0; i < name.size(); ++i)
        lw_name.push_back(tolower(name[i]));
    
    for (const auto& h : headers) {
        if (h.name == lw_name)
            return h.value;
    }

    return string();
}

bool Response::hasHeader(const string& name) const
{
    if (headers.empty())
        return false;
    
    std::string lw_name;
    for (int i = 0; i < name.size(); ++i)
        lw_name.push_back(tolower(name[i]));
    
    for (const auto& h : headers) {
        if (h.name == lw_name)
            return true;
    }
    return false;
}

void Response::setContent(const string& data, const string& content_type)
{
    body = data;
    setHeader("Content-Type", content_type);
}

void Response::setContentProvider(std::size_t len, 
        const std::string& content_type, ContentProvider provider)
{
    // 防止同时设置两个provider
    assert(!content_provider_without_length_);

    assert(len > 0);

    setHeader("Content-Type", content_type);
    content_len_ = len;
    content_provider_ = std::move(provider);
}

void Response::setChunkedContentProvider(const string& content_type, 
            ContentProviderWithoutLength provider)
{
    // 防止同时设置两个provider
    assert(!content_provider_);

    setHeader("Content-Type", content_type);
    content_len_ = 0;
    content_provider_without_length_ = provider;
}

Response Response::stockResponse(const StatusCode& status) {
    Response res;
    res.status = status;
    res.body = status_code::statusToResponseBody(status);
    res.setHeader("Content-Type", "text/html");
    res.setHeader("Content-Length", std::to_string(res.body.size()));
    res.setHeader("Connection", "close");
    return res;
}

} // namespace https_server