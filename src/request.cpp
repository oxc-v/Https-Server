#include "request.hpp"

using std::string;

namespace https_server {

bool Request::hasHeader(const string& name) const
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

string Request::getHeaderValue(const string& name) const
{
    if (headers.empty())
        return string();

    std::string lw_name;
    for (int i = 0; i < name.size(); ++i)
        lw_name.push_back(tolower(name[i]));

    for (const auto& h : headers) {
        if (h.name == lw_name)
            return h.value;
    }
    return string();
}

bool Request::hasFile(const string& key) const
{
    return files.find(key) != files.end();
}

MultipartFormData Request::getFileValue(const string& key) const
{
    auto it = files.find(key);
    if (it != files.end()) { 
        return it->second; 
    }
    return MultipartFormData();
}

bool Request::hasParam(const std::string& key) const
{
    return params.find(key) != params.end();
}


std::string Request::getParamValue(const std::string& key) const
{
    auto it = params.find(key);
    if (it != params.end()) {
        return it->second;
    }

    return std::string();
}

bool Request::isMultipartFormData() const
{
    const auto& content_type = getHeaderValue("Content-Type");
    return !content_type.rfind("multipart/form-data", 0);
}

} // namespace https_server