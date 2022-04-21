#pragma once

#include "service.hpp"
#include "mime_types.hpp"

#include <fmt/format.h>

#include <string>
#include <fstream>
#include <memory>

using std::string;
using std::ifstream;
using namespace https_server;

class WebFileService : public Service {
public:
    virtual void handleRequest(const Request& req, Response& res) override 
    {
        fmt::print("One connection for web_file_servcie: {}\n", req.remote_addr);

        string root_path = "/home/oxc/download/files";
        string file_path = root_path + req.unresolved_path;

        fmt::print("file path: {}\n", file_path);

        auto fin = std::make_shared<ifstream>();
        fin->open(file_path, std::ios::binary | std::ios::ate);
        if (!fin->is_open()) {
            res = Response::stockResponse(StatusCode::not_found);
            return;
        }

        std::size_t file_size = fin->tellg();
        fmt::print("file size: {}\n", file_size);

        string extension = req.unresolved_path.substr(req.unresolved_path.find_last_of(".") + 1);

        // chunk data
        res.setChunkedContentProvider(mime_types::extensionToType(extension),
            [file_size, fin](DataSink& sink) {
                fin->seekg(0); 
                std::size_t offset = 0;
                while (offset < file_size) {
                    char buffer[1024] = {};
                    fin->read(buffer, sizeof(buffer));
                    std::size_t n = fin->gcount();
                    sink.write(buffer, n);
                    offset += n;
                }
                sink.done();
            });
    }
};