#pragma once

#include "service.hpp"
#include "mime_types.hpp"
#include "login.pb.h"

#include <fmt/format.h>
#include <google/protobuf/message.h>
#include <google/protobuf/util/json_util.h>
#include <google/protobuf/text_format.h>

#include <string>

using std::string;
using namespace https_server;
using namespace google::protobuf::util;

class LoginService : public Service {
public:
    virtual void handleRequest(const Request& req, Response& res) override 
    {
        fmt::print("One connection for login_servcie: {}\n", req.remote_addr);
        
        static const string pwd = "123";
        static const string account = "oxc";

        JsonParseOptions parse_opts;
        parse_opts.ignore_unknown_fields = false;
        LoginRequest login_req;
        auto status = JsonStringToMessage(req.body, &login_req, parse_opts);
        if (!status.ok()) {
            res.setContent("params error", "text/plain");
            return;
        }

        LoginResponse login_res;

        if (login_req.account() != account 
                || login_req.password() != pwd) {
            login_res.set_code(0);
            login_res.set_err_msg("Account or password mismatch");
        } else {
            login_res.set_code(1);
            login_res.set_err_msg("Login successful!");
        }

        JsonPrintOptions print_opts;
        print_opts.preserve_proto_field_names = true;
        print_opts.always_print_primitive_fields = true;
        string output;
        MessageToJsonString(login_res, &output, print_opts);

        res.setContent(output, "application/json");
    }
};