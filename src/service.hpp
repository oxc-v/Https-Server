#pragma once

#include "request.hpp"
#include "response.hpp"

namespace https_server {

class Service {
public:
    virtual ~Service() = default;

    virtual void handleRequest(const Request &req, Response &res) = 0;
};

} // namespace https_server