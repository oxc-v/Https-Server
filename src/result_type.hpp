#pragma once

namespace https_server {

// 解析的结果类型
enum ResultType { 
    good,           // 解析正确
    bad,            // 解析错误
    indeterminate   // 表示还有更多的数据等待解析
};

} // namespace https_server