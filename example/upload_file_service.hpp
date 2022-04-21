#pragma once

#include "service.hpp"
#include "mime_types.hpp"

#include <fmt/format.h>

#include <string>
#include <fstream>

using std::string;
using std::ofstream;
using namespace https_server;

class UploadFileService : public Service {
public:
    virtual void handleRequest(const Request& req, Response& res) override 
    {
        fmt::print("One connection for upload_file_servcie: {}\n", req.remote_addr);

        for (const auto& file: req.files) {
            auto filename = file.second.filename;
            auto extension = mime_types::typeToExtension(file.second.content_type);
            auto path = "/home/oxc/download/files/" + filename;
            fmt::print("path: {}\n", path);

            ofstream wf(path, std::ios::out | std::ios::binary);
            if (!wf.is_open()) {
                fmt::print("Could not open: {}\n", filename);
            }
            wf.write(file.second.content.c_str(), file.second.content.size());
            wf.close();
        }

        res.setContent("Upload file successful!", "text/plain");
    }
};