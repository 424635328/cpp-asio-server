[English](README.md) | [中文 (Chinese)](README_zh.md) | [中文简化版](README_lite.md)

# 服务器框架 (Boost.Asio)

一个基于 Boost.Asio 构建的简单高效的服务器框架，方便扩展和定制。

## 特性

*   **高性能:** 使用 Boost.Asio 处理大量连接。
*   **模块化:** 轻松添加自定义功能。
*   **多线程:** 线程池处理连接，速度更快。
*   **HTTP 示例:** 包含 HTTP 服务器示例。
*   **跨平台:** 支持 Windows 和 Linux。
*   **可配置线程池:** 调整线程池大小。
*   **错误处理和日志:** 统一的错误和日志记录。

## 架构

*   **Server:** 监听端口，接受连接。
*   **Connection:** 代表一个客户端连接，负责收发数据。
*   **IOContext:** Boost.Asio 的核心，管理异步操作。
*   **ThreadPool:** 线程池，提高并发能力。
*   **RequestHandler:** 处理客户端请求。
*   **ProtocolHandler:** 处理协议，例如 HTTP。

工作流程： Server 接受连接 -> 创建 Connection -> ThreadPool 分配线程 -> Connection 使用 ProtocolHandler 处理协议 -> RequestHandler 处理请求。

## 快速入门

### 前提条件

*   **Boost 库:** 异步 I/O 和多线程。 使用 Vcpkg (Windows) 或系统包管理器安装。 看 [安装](#安装) 部分。
*   **CMake:** 构建项目。 从 [https://cmake.org/](https://cmake.org/) 下载。
*   **MinGW (Windows, 可选):** 推荐使用。

### 安装

#### 使用 Vcpkg (推荐 Windows)

1.  克隆 Vcpkg: `git clone https://github.com/microsoft/vcpkg.git`
2.  引导 Vcpkg: `cd vcpkg && ./bootstrap-vcpkg.bat` (PowerShell) 或 `./bootstrap-vcpkg.sh` (Git Bash)
3.  集成 Vcpkg: `./vcpkg integrate install`
4.  安装 Boost: `vcpkg install boost:x64-windows` (64 位) 或 `vcpkg install boost:x86-windows` (32 位)

#### 使用 apt (Debian/Ubuntu Linux)

`sudo apt update && sudo apt install libboost-dev libboost-system-dev libboost-thread-dev`

#### 使用 brew (macOS)

`brew install boost`

### 构建

1.  克隆仓库: `git clone https://github.com/424635328/cpp-asio-server && cd cpp-asio-server`
2.  创建构建目录: `mkdir build && cd build`
    *   **重要:** 在 `CMakeLists.txt` 中设置 Vcpkg 工具链:
        `set(CMAKE_TOOLCHAIN_FILE "<vcpkg路径>/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg Toolchain File" FORCE)`

3.  生成构建文件:
    *   MinGW (Windows): `cmake .. -G "MinGW Makefiles"`
    *   Visual Studio (Windows): `cmake -B . -S .. -A x64 -DCMAKE_BUILD_TYPE=Release`
    *   Make (Linux/macOS): `cmake .. -DCMAKE_BUILD_TYPE=Release`

4.  构建项目:
    *   MinGW: `mingw32-make`
    *   Visual Studio: `cmake --build . --config Release`
    *   Make: `make`

### 运行

1.  进入构建目录 (build/ 或 build/Release/)
2.  运行: `./my_server_framework` (Linux/macOS) 或 `./my_server_framework.exe` (Windows)
3.  访问 `http://127.0.0.1:8765`。
4.  验证: `curl http://127.0.0.1:8765` (Linux/macOS) 或 `curl http://127.0.0.1:8765 -o nul` (Windows)

## HTTP 示例

*   `MyHttpServer`: 服务器类，管理连接。
*   `HttpRequestHandler`: 处理 HTTP 请求。
*   `HttpSession`: 管理客户端会话。

代码示例:

```c++
#include "server.h"
#include "http_request_handler.h"
#include "asio_context.h"

int main() {
  size_t num_threads = std::thread::hardware_concurrency();
  AsioContext io_context(num_threads);
  MyHttpServer server(io_context, 8765, 1000);
  io_context.run();
  return 0;
}
```

扩展: 修改 `HttpRequestHandler::handle_request` 添加新功能。

## 配置

*   命令行: `./my_server_framework --port 9000 --max_connections 2000`
*   代码:

```c++
int main(int argc, char* argv[]) {
    short port = vm["port"].as<short>();
    size_t max_connections = vm["max_connections"].as<size_t>();
    MyHttpServer server(*io_context, port, max_connections);
}
```

*   线程池: `size_t num_threads = std::thread::hardware_concurrency(); AsioContext io_context(num_threads);`

## 扩展协议

1.  创建 ProtocolHandler 类:

```c++
class MyProtocolHandler : public ProtocolHandler {
public:
    std::pair<std::string, size_t>  parseRequest(boost::asio::streambuf& buffer) override { /* 解析 */ }
    std::vector<boost::asio::const_buffer>  createResponse(const std::string& response) override { /* 格式化 */ }
};
```

2.  在 Connection 中注册:

```c++
void Connection::start() {
  if (isMyProtocol()) {
    protocol_handler_ = std::make_shared<MyProtocolHandler>();
  } else {
    protocol_handler_ = std::make_shared<HttpProtocolHandler>();
  }
  doRead();
}
```

## 代码示例

*   Echo 服务器:

```c++
class EchoRequestHandler : public RequestHandler {
public:
  HttpResponse handle_request(const HttpRequest& request) override {
    HttpResponse response;
    response.status_code = 200;
    response.body = request.body;
    response.headers["Content-Type"] = "text/plain";
    return response;
  }
};
```

*   静态文件服务器: 放在 web/ 目录下，访问 / 默认提供 web/index.html。

## 代码风格

*   4 个空格缩进
*   类/函数: PascalCase
*   变量: snake_case
*   英文注释

## 贡献

1.  Fork 仓库
2.  创建分支: `git checkout -b feature/my-feature`
3.  提交更改: `git commit -m "添加功能"`
4.  提交 Pull Request

## FAQ

*   **高并发:** 使用异步 I/O 和线程池。
*   **自定义协议:** 实现 ProtocolHandler 接口。
*   **性能测试:** 使用 ab 或 wrk 工具。
*   **VS 构建:** CMake 配置时指定 VS 生成器。

## 性能测试(待续)

*   CPU: Intel Core i7-8700K
*   内存: 16GB DDR4
*   系统: Ubuntu 20.04
*   工具: wrk

| 测试用例 | 并发连接 | RPS   | 延迟 (ms) |
| -------- | -------- | ----- | --------- |
| Hello World| 100      | 10000 | ?         |
| Hello World| 1000     | 80000 | ?         |

命令: `wrk -t12 -c100 -d10s http://127.0.0.1:8765`

## 许可证

MIT 许可证 (LICENSE)

## 联系方式

MiracleHcat@gmail.com