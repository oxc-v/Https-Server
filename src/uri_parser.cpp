#include "uri_parser.hpp"
#include "request_parser.hpp"
#include "request.hpp"

#include <regex>


using std::regex;
using std::regex_match;
using std::smatch;
using std::string;



namespace https_server {

void UriParser::parse(Request& req) 
{
    if (req.uri.empty()) { return; }

    string path_str;
    string params_str;

    RequestParser::split(req.uri, "?", [&](const string& str)
    {
        if (path_str.empty()) {
            path_str = str;

            // 分离path
            static auto re_path = regex(R"((/\w+)((?:/\S+)*))");
            smatch path_sm;
            regex_match(path_str, path_sm, re_path);
            req.path = path_sm[1];
            req.unresolved_path = path_sm[2];
        } else {
            params_str = str;

            RequestParser::split(params_str, "&", [&](const string& s)
            {
                // 提取查询参数
                static auto re_params = regex(R"((\S+)=(\S*))");
                smatch params_sm;
                regex_match(s, params_sm, re_params);
                req.params.emplace(params_sm[1], params_sm[2]);
            });
        }
    });
}

bool UriParser::uriDecode(const string& in, string& out)
{
    out.clear();
    out.reserve(in.size());
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] == '%') {
            if (i + 3 <= in.size()) {
                int value = 0;
                std::istringstream is(in.substr(i + 1, 2));
                if (is >> std::hex >> value) {
                    out += static_cast<char>(value);
                    i += 2;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else if (in[i] == '+') {
            out += ' ';
        } else {
            out += in[i];
        }
    }
    return true;
}

} // namespace https_server
