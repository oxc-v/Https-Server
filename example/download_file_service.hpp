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

class DownloadFileService : public Service {
public:
    virtual void handleRequest(const Request& req, Response& res) override 
    {
        fmt::print("One connection for download_file_servcie: {}\n", req.remote_addr);

        string root_path = "/home/oxc/download/files/";

        // missing paramters
        if (!req.hasParam("filename")) {
            res.status = StatusCode::bad_request;
            res.setContent("missing a parameter: filename.", "text/plain");
            return;
        }

        auto filename = req.getParamValue("filename");
        auto file_path = root_path + filename;
        fmt::print("file_path: {}\n", file_path);

        res.setHeader("Content-Disposition", "attachment; filename=" + filename);

        auto fin = std::make_shared<ifstream>();
        fin->open(file_path, std::ios::binary | std::ios::ate);
        if (!fin->is_open()) {
            res = Response::stockResponse(StatusCode::not_found);
            return;
        }

        // get file size
        std::size_t file_size = fin->tellg();
        fmt::print("{} size: {}\n", filename, file_size);

        // read and send data
        auto extension = filename.substr(filename.find_last_of(".") + 1);
        res.setContentProvider(file_size, mime_types::extensionToType(extension),
            [fin](std::size_t offset, std::size_t length, DataSink& sink)
            {
                fin->seekg(offset);
                while (offset < length) {
                    char buffer[1024] = {};
                    fin->read(buffer, sizeof(buffer));
                    std::size_t n = fin->gcount();
                    sink.write(buffer, n);
                    offset += n;
                }
            }
        );
    }
};