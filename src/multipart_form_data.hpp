#pragma once

#include <string>

struct MultipartFormData {
    std::string name;           // 键值
    std::string content;        // 文件内容
    std::string filename;       // 文件名
    std::string content_type;   // 文件类型
};