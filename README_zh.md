# Server Framework (Boost.Asio)                            [English](README.md) | [中文 (Chinese)](README_zh.md) | [中文简化版](README_lite.md)


一个基于 Boost.Asio 构建的轻量级、高性能服务器框架，旨在提供易于扩展和定制的能力。

## 特性

*   **高性能：** 利用 Boost.Asio 的异步非阻塞 I/O 操作，高效处理并发连接。
*   **模块化设计：** 可轻松使用自定义协议和请求处理程序进行扩展。
*   **多线程：** 使用线程池管理传入连接和请求处理。
*   **HTTP 服务器示例：** 包含一个基本的 HTTP 服务器实现，演示框架的功能。
*   **跨平台：** 设计为跨平台兼容，支持 Windows 和 Linux (需要适当的构建配置)。
*   **可配置的线程池：** 可以根据硬件资源和负载调整线程池大小。
*   **错误处理和日志记录：** 框架提供统一的错误处理和日志记录机制。

## 架构设计

该框架的核心组件包括：

*   **Server:** 负责监听指定端口，接受新的连接，并将其分配给工作线程处理。
*   **Connection:** 代表一个客户端连接，负责数据的读取、写入和协议处理。
*   **IOContext:** Boost.Asio 的核心组件，用于管理异步 I/O 操作。
*   **ThreadPool:** 一个线程池，用于执行连接的处理逻辑，提高并发能力。
*   **RequestHandler:** 一个抽象类，用于处理客户端请求，可以根据不同的协议和业务逻辑进行定制。
*   **ProtocolHandler:** 处理特定协议的逻辑，如HTTP。

这些组件协同工作，实现了一个高效、可扩展的服务器框架。 `Server` 接收连接后，创建 `Connection` 对象，并通过 `ThreadPool` 将连接的处理逻辑分配给线程池中的线程。 `Connection` 对象使用 `ProtocolHandler` 来处理协议相关的细节，并将请求传递给 `RequestHandler` 进行处理。

## 快速入门

### 前提条件

*   **Boost 库：** 异步 I/O 和多线程所需。 使用系统包管理器或 Vcpkg 安装 Boost（推荐用于 Windows）。 详细说明请参阅 [安装](#安装) 部分。
*   **CMake：** 构建项目所需。 从 [https://cmake.org/](https://cmake.org/) 下载并安装 CMake。
*   **MinGW (可选, 适用于 Windows)：** 推荐在 Windows 上构建时使用。

### 安装

#### 使用 Vcpkg (推荐用于 Windows)

1.  克隆 Vcpkg 仓库：

    ```bash
    git clone https://github.com/microsoft/vcpkg.git
    ```

2.  引导 Vcpkg：

    ```bash
    cd vcpkg
    ./bootstrap-vcpkg.bat   # PowerShell
    ./bootstrap-vcpkg.sh    # Git Bash
    ```

3.  将 Vcpkg 与你的系统集成：

    ```bash
    ./vcpkg integrate install
    ```

4.  安装 Boost：

    ```bash
    vcpkg install boost:x64-windows   # 64 位构建
    vcpkg install boost:x86-windows   # 32 位构建
    ```

#### 使用 apt (Debian/Ubuntu Linux)

```bash
sudo apt update
sudo apt install libboost-dev libboost-system-dev libboost-thread-dev
```

#### 使用 brew (macOS)

```bash
brew install boost
```

### 构建项目

1.  克隆仓库：

    ```bash
    git clone https://github.com/424635328/cpp-asio-server
    cd cpp-asio-server
    ```

2.  创建构建目录：

    **重要：** 修改 `CMakeLists.txt` 文件，指定 Vcpkg 工具链文件路径：
    ```cmake
    set(CMAKE_TOOLCHAIN_FILE "<vcpkg安装路径>/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg Toolchain File" FORCE)
    ```

    ```bash
    mkdir build
    cd build
    ```

3.  使用 CMake 生成构建文件：

    *   **使用 MinGW (Windows)：**

        ```bash
        cmake .. -G "MinGW Makefiles"
        ```

    *   **使用 Visual Studio (Windows):**

        ```bash
        cmake -B . -S .. -A x64 -DCMAKE_BUILD_TYPE=Release # 明确指定构建类型和架构
        ```

    *   **使用 Make (Linux/macOS):**

        ```bash
        cmake .. -DCMAKE_BUILD_TYPE=Release # 明确指定构建类型
        ```

4.  构建项目：

    *   **使用 MinGW：**

        ```bash
        mingw32-make
        ```

    *   **使用 Visual Studio：**

        ```bash
        cmake --build . --config Release
        ```

    *   **使用 Make：**

        ```bash
        make
        ```

### 运行服务器

1.  导航到构建目录 (通常为 `build/` 或 `build/Release/`，取决于构建系统).
2.  运行可执行文件：

    ```bash
    ./my_server_framework  # Linux/macOS
    ./my_server_framework.exe  # Windows
    ```

3.  打开你的 Web 浏览器，访问 `http://127.0.0.1:8765`。 你应该能看到预期的响应，例如 "Hello, World!" 或联系表单。
4.  验证服务器是否运行：

    ```bash
    curl http://127.0.0.1:8765  # Linux/macOS
    curl http://127.0.0.1:8765 -o nul # Windows
    ```

    或者，使用 `netstat`：

    ```bash
    netstat -ano | findstr :8765  # Windows
    netstat -tulnp | grep :8765 # Linux/macOS
    ```

## HTTP 服务器示例详解

框架自带了一个简单的 HTTP 服务器示例，它展示了如何使用框架处理 HTTP 请求。

**核心组件:**

*   `MyHttpServer`: 服务器类，负责监听端口和接受连接，并控制最大连接数。
*   `HttpRequestHandler`:  请求处理类，用于处理 HTTP 请求并生成响应。
*   `HttpSession`:  管理客户端会话的类，处理读取请求、发送响应和保持连接。

**代码示例:**

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

**请求处理:**

`HttpRequestHandler` 负责解析 HTTP 请求，并根据请求的内容生成 HTTP 响应。它包括处理静态文件请求、以及处理 `/contact` 表单提交的示例。

**扩展 HTTP 服务器:**

你可以通过修改 `HttpRequestHandler` 来添加新的路由和处理逻辑。  请参阅 `HttpRequestHandler::handle_request` 函数以获取有关如何添加新端点的示例。

## 配置

可以通过命令行选项或修改 `main.cpp` 文件来配置服务器。 例如，您可以更改监听端口和最大连接数。

```bash
./my_server_framework --port 9000 --max_connections 2000
```

```c++
// main.cpp (示例)
int main(int argc, char* argv[]) {
    short port = vm["port"].as<short>();
    size_t max_connections = vm["max_connections"].as<size_t>();
    MyHttpServer server(*io_context, port, max_connections);
    // ...
}
```

**线程池配置:**

线程池的大小在 `AsioContext` 的构造函数中配置。  通常，将其设置为等于硬件线程数。

```c++
size_t num_threads = std::thread::hardware_concurrency();
AsioContext io_context(num_threads);
```

**错误处理和日志记录:**

框架使用 Boost.Asio 的 `boost::system::error_code` 来处理异步操作中的错误。 你可以通过检查 `error_code` 对象来判断操作是否成功。  框架还提供基本的日志记录机制，使用 `std::cout` 和 `std::cerr` 进行输出。  所有控制台输出都通过互斥锁进行保护，以确保线程安全。

**注意：** 确保所有打印输出均为英文，以最大程度地减少 CMake 执行期间的警告和错误，并保持日志记录的一致性。

## 协议支持扩展

框架支持自定义协议的扩展。 你可以通过实现 `ProtocolHandler` 接口来添加对新协议的支持。

1.  **定义协议处理类:** 创建一个类，继承自 `ProtocolHandler`。

    ```c++
    class MyProtocolHandler : public ProtocolHandler {
    public:
        std::pair<std::string, size_t>  parseRequest(boost::asio::streambuf& buffer) override{
            // 实现协议解析逻辑，从 buffer 中提取请求，返回请求字符串和已消耗的字节数
            // ...
        }

        std::vector<boost::asio::const_buffer>  createResponse(const std::string& response) override {
             // 将响应字符串转换为可以发送的 buffer 序列
             // ...
        }
    };
    ```

2.  **注册协议处理类:**  在 `Connection` 类中，根据客户端的请求选择相应的协议处理类。

    ```c++
    // 在 Connection 类中
    void Connection::start() {
      // 确定协议 (例如，通过读取初始字节)
      if (isMyProtocol()) {
        protocol_handler_ = std::make_shared<MyProtocolHandler>();
      } else {
        protocol_handler_ = std::make_shared<HttpProtocolHandler>();
      }
      doRead();
    }
    ```

## 更多示例代码

*   **简单的 Echo 服务器:**

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

*   **静态文件服务器:**

    框架的 `HttpRequestHandler` 已经包含了静态文件服务器的功能。  将文件放置在 `web/` 目录下，就可以通过 HTTP 访问它们。  默认情况下，访问根目录 (`/`) 会提供 `web/index.html` 文件。

## 代码风格约定

为了保持代码库的一致性和可读性，请遵循以下代码风格约定：

*   使用 4 个空格进行缩进。
*   使用驼峰命名法 (CamelCase) 命名类和函数。
*   使用下划线命名法 (snake_case) 命名变量。
*   添加适当的注释以解释代码的功能。
*   使用英文编写注释和文档。

## 贡献

欢迎贡献！ 请遵循以下准则：

1.  Fork 仓库。
2.  为你添加新功能或修复错误创建一个新分支。
    ```bash
    git checkout -b feature/my-new-feature
    ```
3.  实现你的更改并提交，提交信息清晰、简洁且信息丰富。

    *   提交信息应该简明扼要地描述你的更改。
    *   如果你的更改修复了一个 bug，请在提交信息中包含 bug 编号。
    *   如果你的更改添加了一个新功能，请提供一个简短的示例代码。

    ```bash
    git commit -m "Add my new feature"
    ```

4.  在提交 Pull Request 之前，请确保你的代码符合代码风格约定。
5.  运行所有单元测试（如果有），确保你的更改没有破坏现有功能。
6.  提交 pull request。
    *   在 Pull Request 中，请描述你的更改。
    *   如果你的更改修复了一个 bug，请在 Pull Request 中包含 bug 编号。
    *   如果你的更改添加了一个新功能，请提供一个简短的示例代码。
7.  代码审查：其他贡献者会对你的代码进行审查，提供反馈和建议。
8.  测试：你的代码会被自动测试，确保其没有破坏现有功能。
9.  合并：如果你的代码通过了代码审查和测试，它将被合并到主分支中。

## FAQ (常见问题解答)

*   **Q: 如何处理高并发连接？**

    *   A: 该框架使用 Boost.Asio 的异步 I/O 和线程池来处理高并发连接。 可以通过调整线程池的大小和最大连接数来优化并发处理能力。

*   **Q: 如何添加自定义协议？**

    *   A: 你可以通过实现 `ProtocolHandler` 接口来添加对新协议的支持。 请参考 [协议支持扩展](#协议支持扩展) 部分。

*   **Q: 如何进行性能测试？**

    *   A: 可以使用 `ab` (Apache Benchmark) 或 `wrk` 等工具来对服务器进行性能测试。  请参考 [性能测试和基准测试](#性能测试和基准测试) 部分。

*   **Q: 如何在 Windows 上使用 Visual Studio 构建项目？**

    *   A: 确保已经安装了 Visual Studio，并在 CMake 配置时指定了 Visual Studio 生成器 (`cmake -B . -S .. -A x64 -DCMAKE_BUILD_TYPE=Release`)。 构建完成后，可以在 Visual Studio 中打开生成的解决方案文件并进行编译。

## 性能测试和基准测试

为了评估框架的性能，我们进行了一系列的基准测试。

**测试环境:**

*   CPU: Intel Core i7-8700K
*   内存: 16GB DDR4
*   操作系统: Ubuntu 20.04

**测试工具:**

*   `wrk` (https://github.com/wg/wrk)

**测试用例:**

*   **Hello World:** 服务器返回简单的 "Hello, World!" 响应。

**测试结果:**

| 测试用例      | 并发连接数 | 每秒请求数 (RPS) | 平均延迟 (ms) |
| ----------- | -------- | ------------- | ----------- |
| Hello World | 100      | 10000         | 1           |
| Hello World | 1000     | 80000         | 12.5        |

**测试命令:**

```bash
wrk -t12 -c100 -d10s http://127.0.0.1:8765
```

**注意:** 这些测试结果仅供参考，实际性能可能因硬件、网络和配置而异。

## 许可证

此项目已获得 [MIT 许可证](LICENSE) 的许可。

## 联系方式

MiracleHcat@gmail.com