# HTTPS-Server

一个基于 [Asio](https://think-async.com/Asio/) 实现的HTTPS库。

它使用 [C++20 Coroutines](https://en.cppreference.com/w/cpp/language/coroutines) 与 [Asio asynchronous model](https://www.boost.org/doc/libs/1_79_0/doc/html/boost_asio/overview/model.html) 实现高并发。

注意:这是一个基于多线程“半阻塞”(读数据是异步,写数据是阻塞)的库。

# 简单示例

Echo "Hello, world. " 服务端代码。

```cpp
#include "server.hpp"
#include "service.hpp"

using namespace https_server;

class EchoService : public Service
{
public:
    virtual void handleRequest(const Request& req, Response& res) override
    {
        res.setContent("Hello, world.", "text/plain");
    }
};

int main()
{
    EchoService echo_service;

    // 相关配置
    Option opt;
    opt.setCrtFilePath("path");        // 证书路径
    opt.setPrivateKeyFilePath("path"); // 私钥路径
    opt.setPrivateKeyPwd("pwd");       // 私钥密码

    // 线程池
    std::size_t io_ctx_pool_size = 8;

    Server s("localhost", 8888, io_ctx_pool_size, opt);
    
    // 注册服务
    s.addService("/EchoService", echo_service);

    // 运行服务
    s.run();    // 阻塞等待服务器终止

    return 0;
}
```

Https-Server目前支持`HEAD`，`GET`，`POST`方法，其它请求方法还在进一步开发中...

# 处理'multipart/form-data'数据

`multipart/form-data`常用于客户端向服务端上传文件，以下演示如何处理文件上传：

```cpp
class FileService : public Service
{
public:
    virtual void handleRequest(const Request& req, Response& res)
    {
        static const string root_path = "/home/oxc/download/files/";

        // 接受多个文件
        for (const auto& file: req.files) {
            auto filename = file.second.filename;            
            auto extension = mime_types::typeToExtension(file.second.content_type);            
            auto path = root_path + filename;

            // 将文件写入硬盘                  
            ofstream wf(path, std::ios::out | std::ios::binary);
            wf.write(file.second.content.c_str(), file.second.content.size());            
            wf.close();
        }
    }
};
```

# 使用content provider发送数据

HTTPS-Server支持接收`Range`形式的请求方式，使用content provider发送数据时，将自动处理范围请求。即客户端Range包含多个范围，content provider将调用多次，其中`offset`表示请求的偏移量，`length`表示该范围的长度。

注意：使用content provider发送数据时，将不对reposne body进行压缩处理。

```cpp
class EchoService : public Service
{
public:
    virtual void handleRequest(const Request& req, Response& res)
    {
        const string data = "Hello, world.";
        const std::size_t chunk_size = 5;

        res.setContentProvider(
            data.size(),  // 数据长度
            "text/plain", // 数据类型
            [data, chunk_size](std::size_t offset, std::size_t length, DataSink& sink) {
                // 分块读取和发送数据，可减轻内存压力            
                while (offset < length) {
                    std::size_t write_n = std::min(chunk_size, length - offset);
                    sink.write(&data[offset], write_n);
                    offset += write_n;
                }
            }
        );
    }
};
```

# 分块传输

使用`chunk`方式传输数据不用指定数据的具体长度，适用于在不知晓数据的具体长度的情况使用。

```cpp
class EchoService : public Service
{
public:
    virtual void handleRequest(const Request& req, Response& res)
    {
        const string data = "Hello, world.";
        const std::size_t chunk_size = 5;

        res.setChunkedContentProvider(
            "text/plain", // 数据类型
            [data, chunk_size](DataSink& sink) {
                std::size_t offset = 0;          
                while (offset < data.size()) {
                    std::size_t write_n = std::min(chunk_size, data.size() - offset);
                    sink.write(&data[offset], write_n);
                    offset += write_n;
                }
                sink.done(); // 0/r/n/r/n
            }
        );
    }
};
```

# HTTP参数

## HTTP headers

```cpp
// 获得header中的"Content-Type"的值，大小写不敏感。
// request中不存在"Content-Type"时将返回空字符串。
const std::string content_type = req.getHeader("Content-Type");
if (!content_type.empty()) {
    fmt::print("Content-Type is {}\n", content_type);
}

// 设置response的header，大小写不敏感。
res.setHeader("Content-Type", "text/plain");

// 覆盖header
res.setHeader("Content-Type", "application/json");

// 追加一个value，每个value用逗号分隔。
// 如："Accept-encoding: gzip,br"。
res.setHeader("Accept-encoding", "gzip");
res.appendHeader("Accept-encoding", "br");
```

## Content-Type

Content-Type是一个使用频率较高的header，在一些文件服务中尤为重要。HTTPS-Server内置了一些常见MIME-Type，并提供一个根据文件扩展名转换为MIME-Type的方法`string mime_types::extensionToType(const string& extension)`。

```cpp
const std::string filename = "hello.txt";
auto extension = filename.substr(filename.find_last_of(".") + 1);
auto mime_type = mime_types::extensionToType(extension);
```

以下是内置的一些MIME-Type：

| Extension | MIME Type                | Extension | MIME Type                   |
|:--------- |:------------------------ |:--------- |:--------------------------- |
| css       | text/css                 | mp3       | audio/mp3                   |
| csv       | text/csv                 | mpeg      | video/mpeg                  |
| txt       | text/plain               | mp4       | video/mp4                   |
| vtt       | text/vtt                 | xml       | application/xml             |
| html, htm | text/html                | gz        | application/gzip            |
| apng      | image/apng               | zip       | application/zip             |
| avif      | image/avif               | mpga      | audio/mpeg                  |
| bmp       | image/bmp                | wav       | audio/wave                  |
| gif       | image/gif                | ttf       | font/ttf                    |
| png       | image/png                | woff      | font/woff                   |
| svg       | image/svg+xml            | woff2     | font/woff2                  |
| webp      | image/webp               | 7z        | application/x-7z-compressed |
| ico       | image/vnd.microsoft.icon | pdf       | application/pdf             |
| tif       | image/tiff               | js        | application/javascript      |
| tiff      | image/tiff               | json      | application/json            |
| jpeg, jpg | image/jpeg               | tar       | application/x-tar           |

## Status Code

一般在设置了response的content后，HTTPS-Server将自动为response填充相应的状态码，这些值定义在[status_code.hpp](./src/status_code.hpp)中。

```cpp
// 根据状态码返回相应的response结构
auto res = response.stockResponse(StatusCode::not_found);
```

## Query String

查询参数是`URL`的一部分，它的常见形式为`key1=value1&key2=value2&...`，查询参数的具体格式不是HTTP规范的一部分，这里只采用常用的格式。

注意：HTTPS-Server不识别查询参数之间的空格，即`key1 = value&...`将不被成功解析。

```cpp
// 获取request的某个查询参数
if (req.hasParam("filename")) {
    fmt::print("filename = {}\n", req.getParamValue("filename"));
}
```

# 压缩response body

HTTPS-Server支持 `gzip`和`br`两种压缩方式，默认使用br压缩数据。

设置`Option::setEncodingType(EncodingType::Brotli)`后将采用尝试`br`压缩body。当request中没有包含`Accept-encoding`或 Accept-encoding中不包含br时，将不对body进行压缩。

注意：暂不支持`解压request body`。

# 协议支持

目前只支持HTTP/1.1协议，即默认支持长连接。

# 构建项目

## 支持环境

- Ubuntu/WSL

## 编译器

- GCC 10以上。

## 依赖

- [Asio](https://think-async.com/Asio/)

- [Fmtlib](https://github.com/fmtlib/fmt)

- [Brotli](https://brotli.org)


