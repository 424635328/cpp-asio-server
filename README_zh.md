# 服务器框架 (Boost.Asio)

一个基于 Boost.Asio 构建的轻量级、高性能服务器框架，旨在提供易于扩展和定制的能力。

## 特性

*   **高性能:** 利用 Boost.Asio 的异步非阻塞 I/O 操作，高效处理并发连接。  异步 I/O 允许服务器在等待 I/O 操作完成时处理其他任务，从而提高了吞吐量和响应速度.
*   **模块化设计:** 可轻松使用自定义协议和请求处理程序进行扩展。  通过实现 `ProtocolHandler` 和 `RequestHandler` 接口，您可以轻松添加对新协议和请求类型的支持.
*   **多线程:** 使用线程池管理传入连接和请求处理，最大限度地提高吞吐量。  线程池减少了创建和销毁线程的开销，从而提高了性能.
*   **HTTP 服务器示例:** 包含一个基本的 HTTP 服务器实现，开箱即用地演示框架的功能。  HTTP 服务器示例演示了如何处理 HTTP 请求、提供静态文件和处理表单提交.
*   **跨平台:** 设计为跨平台兼容，支持 Windows 和 Linux (需要适当的构建配置)。  该框架可以使用 CMake 构建，CMake 是一个跨平台构建系统.
*   **可配置的线程池:** 可以根据硬件资源和预期工作负载调整线程池大小。  线程池的大小应该根据 CPU 核心数和预期的并发连接数进行调整.
*   **错误处理和日志记录:** 在整个框架中提供一致的错误处理和日志记录机制。  框架使用 Boost.Asio 的 `boost::system::error_code` 进行错误处理，并使用 `std::cout` 和 `std::cerr` 进行日志记录.
*   **拓展：** 提供wasm模块进行拓展功能(例如:[gray_scale.cpp](gray_scale.cpp))，提升性能。  WASM 模块允许您使用其他编程语言（如 C++）编写高性能代码，并在 Web 浏览器中运行它.

## 架构概览

框架的核心组件包括：

*   **Server (服务器):** 监听指定的端口，接受新的连接，并将它们分派给工作线程。 服务器使用 `boost::asio::acceptor` 监听连接请求.
*   **Connection (连接):** 代表一个客户端连接，负责读取数据、写入数据，以及处理协议特定的细节。 `Connection` 类管理连接的生命周期，并处理部分读取和写入.
*   **IOContext:** Boost.Asio 的核心组件，管理异步 I/O 操作。 可以将其视为驱动所有异步任务的引擎。  `io_context` 管理事件循环和完成处理程序.
*   **ThreadPool (线程池):** 用于执行连接处理逻辑的线程池，提高并发性和响应能力。 线程被重用于不同的连接，从而减少开销。 `ThreadPool` 减少了创建和销毁线程的开销，从而提高了性能.
*   **RequestHandler (请求处理程序):** 一个抽象类（接口），定义了如何处理客户端请求。 您需要实现具体的 `RequestHandler` 类来处理特定的请求类型或业务逻辑。 `RequestHandler` 接口定义了 `handle_request()` 方法，该方法用于处理客户端请求并返回 HTTP 响应.
*   **ProtocolHandler (协议处理程序):** 处理特定协议的细节（例如，HTTP）。 负责根据协议规则解析传入数据和格式化传出数据。 `ProtocolHandler` 接口定义了 `parseRequest()` 和 `createResponse()` 方法，分别用于解析请求和格式化响应.

这些组件协同工作，提供了一个高效且可扩展的服务器框架。 `Server` 接受传入的连接，为每个连接创建一个 `Connection` 对象，然后使用 `ThreadPool` 分配一个线程来处理连接的处理。 `Connection` 对象使用 `ProtocolHandler` 来管理协议相关的细节（如 HTTP 头部），并将处理后的请求传递给 `RequestHandler` 以执行业务逻辑。

## 快速入门

### 前提条件

*   **Boost 库:** 异步 I/O 和多线程所需。 使用系统的包管理器或 Vcpkg 安装 Boost (推荐用于 Windows)。 有关详细说明，请参阅 [安装](#安装) 部分。
*   **CMake:** 构建项目所需。 从 [https://cmake.org/](https://cmake.org/) 下载并安装 CMake。
*   **MinGW (可选, Windows):** 推荐在 Windows 上使用 MinGW 工具链进行构建。

### 安装

#### 使用 Vcpkg (推荐用于 Windows)

Vcpkg 是 Windows 上 C++ 库的包管理器，简化了安装过程。

1.  克隆 Vcpkg 仓库：

    ```bash
    git clone https://github.com/microsoft/vcpkg.git
    ```

2.  引导 Vcpkg: 这将准备 Vcpkg 以在您的系统上使用。

    ```bash
    cd vcpkg
    ./bootstrap-vcpkg.bat   # PowerShell
    ./bootstrap-vcpkg.sh    # Git Bash
    ```

3.  将 Vcpkg 与您的系统集成： 这设置 Vcpkg 以供 CMake 使用。

    ```bash
    ./vcpkg integrate install
    ```

4.  安装 Boost： 这将下载并构建 Boost 库。 选择正确的架构（x64 用于 64 位构建，x86 用于 32 位）。

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

2.  创建一个构建目录： 最好在单独的目录中进行构建，以保持源代码的整洁。

    **重要:** 在生成构建文件之前，您 **必须** 告诉 CMake 在哪里可以找到 Vcpkg。 修改 `CMakeLists.txt` 文件以指定 Vcpkg 工具链文件路径：

    ```cmake
    set(CMAKE_TOOLCHAIN_FILE "<vcpkg安装路径>/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg Toolchain File" FORCE)
    ```
    将 `<vcpkg安装路径>` 替换为 Vcpkg 安装的实际路径。

3.  使用 CMake 生成构建文件： 使用的命令取决于您的构建环境。

    *   **使用 MinGW (Windows):**

        ```bash
        cmake .. -G "MinGW Makefiles"
        ```

    *   **使用 Visual Studio (Windows):**

        ```bash
        cmake -B build -S . -A x64 -DCMAKE_BUILD_TYPE=Release  # 明确指定架构和构建类型
        ```

    *   **使用 Make (Linux/macOS):**

        ```bash
        cmake .. -DCMAKE_BUILD_TYPE=Release  # 明确指定构建类型
        ```

4.  构建项目：

    *   **使用 MinGW:**

        ```bash
        mingw32-make
        ```

    *   **使用 Visual Studio:**

        ```bash
        cmake --build . --config Release
        ```

    *   **使用 Make:**

        ```bash
        make
        ```

### 运行服务器

1.  导航到构建目录 (通常为 `build/` 或 `build/Release/`，具体取决于您的构建系统)。
2.  运行可执行文件：

    ```bash
    ./my_server_framework  # Linux/macOS
    ./my_server_framework.exe  # Windows
    ```

3.  打开您的 Web 浏览器并访问 `http://127.0.0.1:8765`。您应该会看到预期的响应 (例如，"Hello, World!" 或联系表单)。

    **如果出现端口被占用的情况：**  您可以尝试修改 `main.cpp` 文件中的端口号，或者关闭占用该端口的进程.  您可以使用 `netstat -ano | findstr :8765` (Windows) 或 `netstat -tulnp | grep :8765` (Linux/macOS) 命令来查找占用该端口的进程.

4.  验证服务器是否正在运行：

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

该框架包含一个简单的 HTTP 服务器示例，演示了如何处理 HTTP 请求。

**关键组件：**

*   `MyHttpServer`: 服务器类，负责监听连接、接受连接以及管理最大并发连接数。  `MyHttpServer` 继承自 `Server` 类，并重写了 `create_session()` 方法，用于创建 `HttpSession` 对象.
*   `HttpRequestHandler`: 处理逻辑以处理 HTTP 请求并生成适当响应。 这是定义服务器端点和行为的地方。 `HttpRequestHandler` 类实现了 `handle_request()` 方法，该方法用于处理客户端请求并返回 HTTP 响应.  `handle_request()` 方法可以处理静态文件请求、表单提交和 API 请求.
*   `HttpSession`: 管理单个客户端会话，处理读取请求、发送响应以及潜在地保持连接活动（HTTP 持久连接）。  `HttpSession` 类负责读取 HTTP 请求、调用 `HttpRequestHandler` 处理请求、发送 HTTP 响应，以及关闭连接.

**代码示例：**

```c++
#include "server.h"
#include "http_request_handler.h"
#include "asio_context.h"

int main() {
  // 确定可用的硬件线程数。
  size_t num_threads = std::thread::hardware_concurrency();

  // 创建一个具有确定大小的线程池的 AsioContext。 这管理着异步任务的执行。
  AsioContext io_context(num_threads);

  // 创建 HTTP 服务器，指定 io_context、要侦听的端口 (8765) 和最大连接数 (1000)。
  MyHttpServer server(io_context, 8765, 1000);

  // 启动 io_context，开始处理异步任务。
  io_context.run();

  return 0;
}
```

**请求处理：**

`HttpRequestHandler` 负责解析 HTTP 请求并根据其内容生成响应。 这包括处理静态文件请求（提供 HTML、CSS、JavaScript、图像）以及处理示例 `/contact` 表单提交。

**扩展 HTTP 服务器：**

您可以通过修改 `HttpRequestHandler` 以添加新路由和处理逻辑来轻松扩展 HTTP 服务器。 请参阅 `HttpRequestHandler::handle_request` 函数，以获取有关如何添加新端点并定义其行为的示例。

**示例：添加新的路由**

```c++
HttpResponse HttpRequestHandler::handle_request(const HttpRequest& request) {
    cout << "收到请求: " << request.method << " " << request.uri << endl;

    if (request.uri == "/api/data" && request.method == "GET") {
        // 处理 /api/data 请求
        HttpResponse response;
        response.status_code = 200;
        response.headers["Content-Type"] = "application/json";
        response.body = "{\"message\": \"Hello from API\"}";
        return response;
    } else if (request.uri == "/contact" && request.method == "POST") {
        // ... (处理联系表单提交) ...
    } else {
        return handle_static_file_request(request);
    }
}
```

**关键代码解释：**

*   **线程池的创建和管理：**  `AsioContext` 类负责创建和管理线程池.  线程池的大小应该根据 CPU 核心数和预期的并发连接数进行调整.  过多的线程会导致上下文切换开销，而过少的线程会导致吞吐量降低.  一般来说，线程池的大小设置为 CPU 核心数的 2 倍是一个不错的选择.
*   **异步 I/O 的使用：**  `HttpSession` 类使用 Boost.Asio 的异步 I/O 操作来读取和写入数据.  异步 I/O 允许服务器在等待 I/O 操作完成时处理其他任务，从而提高了吞吐量和响应速度.
*   **请求处理器的实现：**  `HttpRequestHandler` 类实现了 `handle_request()` 方法，该方法用于处理客户端请求并返回 HTTP 响应.  `handle_request()` 方法可以处理静态文件请求、表单提交和 API 请求.
*   **HTTP 状态码的使用：**  `HttpRequestHandler` 类使用 HTTP 状态码来指示请求的结果.  例如，`200 OK` 表示请求成功，`404 Not Found` 表示请求的资源不存在.

## 配置

可以通过命令行选项或修改 `main.cpp` 文件来配置服务器。 您可以更改侦听端口、最大连接数以及其他参数。

```bash
./my_server_framework --port 9000 --max_connections 2000
```

```c++
// main.cpp（示例）
#include <boost/program_options.hpp> // 引入 boost program_options

namespace po = boost::program_options;

int main(int argc, char* argv[]) {
    try {
        // 定义命令行选项
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message") // 帮助信息
            ("port", po::value<short>()->default_value(8765), "set listening port") // 设置监听端口
            ("max_connections", po::value<size_t>()->default_value(std::numeric_limits<size_t>::max()), "set max connections"); // 设置最大连接数

        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm); // 解析命令行参数
        po::notify(vm);

        if (vm.count("help")) {
            cout << desc << endl;
            return 0;
        }

        short port = vm["port"].as<short>(); // 获取端口号
        size_t max_connections = vm["max_connections"].as<size_t>(); // 获取最大连接数

        // ... (创建 AsioContext 和 MyHttpServer) ...

    } catch (exception& e) {
        cerr << "异常: " << e.what() << endl;
        return 1;
    }

    return 0;
}
```

**Boost.Program_options 使用说明：**

*   **定义选项：** 使用 `po::options_description` 类定义命令行选项.  每个选项都有一个名称、一个描述和一个类型.
*   **解析命令行：** 使用 `po::store()` 函数解析命令行参数，并将结果存储在 `po::variables_map` 对象中.
*   **访问选项：** 使用 `vm["option_name"].as<type>()` 方法访问选项的值.

**线程池配置：**

线程池的大小在 `AsioContext` 的构造函数中配置。 通常，将其设置为等于可用的硬件线程数，以获得最佳性能。

```c++
size_t num_threads = std::thread::hardware_concurrency();
AsioContext io_context(num_threads);
```

**线程池大小的权衡：**

*   **过多的线程：**  会导致上下文切换开销增加，从而降低性能.
*   **过少的线程：**  会导致吞吐量降低，因为服务器无法充分利用 CPU 资源.

**建议的线程池大小计算方法：**

*   `线程数 = CPU 核心数 * 2`

**错误处理和日志记录：**

该框架利用 Boost.Asio 的 `boost::system::error_code` 在异步操作中实现一致的错误处理。 在每次异步操作之后，检查 `error_code` 对象以确定操作是否成功。 使用 `std::cout` 和 `std::cerr` 提供基本日志记录，所有控制台输出都受到互斥锁的保护，以确保线程安全。

**注意：** 确保所有控制台输出（包括日志消息）均为英文，以最大程度地减少 CMake 执行期间的潜在警告和错误，并保持一致性。  `AsioContext::cout_mutex` 用于保护 `std::cout` 和 `std::cerr` 的访问，以避免多线程环境下的竞争条件.  不正确使用互斥锁可能会导致死锁，请确保始终以相同的顺序获取互斥锁.

**示例：错误处理**

```c++
boost::asio::async_read(socket_, buffer_, boost::asio::transfer_at_least(1),
    [this](boost::system::error_code ec, size_t bytes_transferred) {
        if (ec) {
            std::cerr << "Error reading from socket: " << ec.message() << std::endl;
            stop();
            return;
        }
        // ... (处理读取的数据) ...
    });
```

## 扩展协议支持

该框架允许您通过实现 `ProtocolHandler` 接口来添加对自定义协议的支持。

1.  **定义一个协议处理程序类：** 创建一个继承自 `ProtocolHandler` 的类。 此类将包含用于解析和生成协议特定数据的逻辑。

    ```c++
    class MyProtocolHandler : public ProtocolHandler {
    public:
        // 实现协议解析逻辑。 从缓冲区中提取请求，返回请求字符串和已消耗的字节数。
        std::pair<std::string, size_t>  parseRequest(boost::asio::streambuf& buffer) override {
            // ... (协议解析逻辑) ...
        }

        // 将响应字符串转换为可以在套接字上发送的一系列缓冲区。
        std::vector<boost::asio::const_buffer>  createResponse(const std::string& response) override {
            // ... (响应格式化逻辑) ...
        }
    };
    ```

2.  **注册协议处理程序：** 在 `Connection` 类中，确定使用的协议 (例如，通过检查连接的初始字节)，然后选择适当的 `ProtocolHandler` 实现。

    ```c++
    // 在 Connection 类中：
    void Connection::start() {
      // 确定协议（例如，通过读取初始字节）
      if (isMyProtocol()) {
        protocol_handler_ = std::make_shared<MyProtocolHandler>();
      } else {
        protocol_handler_ = std::make_shared<HttpProtocolHandler>();
      }
      doRead();  // 开始使用选定的协议处理程序从套接字读取数据。
    }
    ```

## 更多代码示例

*   **简单的回显服务器：**

    ```c++
    class EchoRequestHandler : public RequestHandler {
    public:
      HttpResponse handle_request(const HttpRequest& request) override {
        HttpResponse response;
        response.status_code = 200;
        response.body = request.body; // 将请求正文回显到客户端。 无论客户端发送什么，服务器都会发回什么。
        response.headers["Content-Type"] = "text/plain";
        return response;
      }
    };
    ```

*   **静态文件服务器：**

    该框架的 `HttpRequestHandler` 已经包含静态文件服务功能。 将您的文件放在 `web/` 目录中，它们将可以通过 HTTP 访问。 默认情况下，访问根路径 (`/`) 将提供 `web/index.html` 文件。

    **目录结构：**

    ```
    cpp-asio-server/
    ├── web/
    │   ├── index.html
    │   ├── style.css
    │   └── script.js
    ├── src/
    │   ├── ...
    ├── ...
    ```

## 代码风格约定

为了保持代码库中的一致性和可读性，请遵循以下代码风格约定：

*   使用 4 个空格进行缩进。
*   对于类和函数名称，使用 PascalCase（也称为大驼峰式命名）(例如，`MyClass`，`CalculateValue`)。
*   对于变量名，使用 snake_case (例如，`my_variable`，`user_name`)。
*   添加适当的注释来解释代码的目的和功能。 专注于代码 *为什么* 要执行它所执行的操作，而不仅仅是代码 *做什么*。
*   用英语编写注释和文档。
*   使用代码格式化工具（例如，clang-format）来自动格式化代码，以确保代码库中的一致性。

## 贡献

欢迎贡献！ 请遵循以下准则：

1.  Fork 该仓库。
2.  为您的功能或错误修复创建一个新分支。 为分支提供一个描述性名称。
    ```bash
    git checkout -b feature/my-new-feature
    ```
3.  实现您的更改并提交它们，并使用清晰、简洁和信息丰富的提交消息。

    *   提交消息应概括提交中所做的更改。
    *   如果您的更改修复了一个错误，请在提交消息中包含错误编号。
    *   如果您的更改添加了一个新功能，请提供一个简短的代码示例，演示该功能的用法。
    *   遵循 [Conventional Commits](https://www.conventionalcommits.org/zh-cn/v1.0.0/) 规范，以使提交历史更加清晰和一致。

    ```bash
    git commit -m "feat: 添加我的新功能：实现了 X、Y 和 Z"
    ```

4.  在提交拉取请求之前，请确保您的代码符合代码风格约定。
5.  运行所有单元测试（如果适用），以验证您的更改是否没有引入回归。
6.  提交拉取请求。

    *   在拉取请求说明中，提供您所做更改的详细说明。
    *   如果您的更改修复了一个错误，请在拉取请求说明中包含错误编号。
    *   如果您的更改添加了一个新功能，请提供一个简短的代码示例，演示在拉取请求说明中该功能的用法。 这使审阅者更容易理解您的更改。

7.  代码审查：其他贡献者将审查您的代码，提供反馈并提出改进建议。 准备好处理收到的任何反馈。
8.  测试：您的代码将被自动测试，以确保它不会破坏现有功能。
9.  合并：一旦您的代码通过代码审查和测试，它将被合并到主分支中。

## 常见问题解答 (FAQ)

*   **Q: 如何处理大量并发连接？**

    *   A: 该框架使用 Boost.Asio 的异步 I/O 和线程池来有效管理高并发。 为了提高容量，您可以调整线程池大小和允许的最大连接数。 关键是在您的请求处理程序中避免阻塞操作。 避免在请求处理程序中执行耗时操作，例如，数据库查询或文件 I/O.  您可以将这些操作放入单独的线程中，或者使用异步 API 来执行它们.

*   **Q: 如何添加对自定义协议的支持？**

    *   A: 实现 `ProtocolHandler` 接口，并在 `Connection` 类中注册它。 有关详细信息，请参阅 [扩展协议支持](#扩展协议支持) 部分。

*   **Q: 如何执行性能测试？**

    *   A: 使用诸如 `ab` (Apache Benchmark) 或 `wrk` 之类的工具来对服务器进行基准测试。 请参阅 [性能测试和基准测试](#性能测试和基准测试) 部分。

*   **Q: 如何使用 Visual Studio 在 Windows 上构建项目？**

    *   A: 确保您已安装 Visual Studio，并在 CMake 配置期间指定了 Visual Studio 生成器 (例如，`cmake -B . -S .. -A x64 -DCMAKE_BUILD_TYPE=Release`)。 生成构建文件后，您可以在 Visual Studio 中打开生成的解决方案文件 (`.sln`) 并构建项目。

*   **Q: 如何支持 HTTPS？**

    *   A:  要支持 HTTPS，您需要使用 `boost::asio::ssl::context` 类来创建一个 SSL 上下文，并将该上下文传递给 `boost::asio::ssl::stream` 对象.  您还需要配置服务器以使用 TLS/SSL 证书.  以下是一些步骤：
        1.  **生成 TLS/SSL 证书：** 您可以使用 OpenSSL 或其他工具生成 TLS/SSL 证书.
        2.  **创建 SSL 上下文：** 使用 `boost::asio::ssl::context` 类创建一个 SSL 上下文，并配置该上下文以使用您的证书.
        3.  **创建 SSL 流：** 使用 `boost::asio::ssl::stream` 类创建一个 SSL 流，并将该流与您的套接字和 SSL 上下文关联.
        4.  **执行握手：** 使用 `boost::asio::ssl::stream::async_handshake` 方法执行握手.
        5.  **读取和写入数据：** 使用 `boost::asio::async_read` 和 `boost::asio::async_write` 方法读取和写入数据.

*   **Q: 如何防止 CSRF 攻击？**
     *   A:  要防止 CSRF 攻击，您可以使用以下方法：
        1.  **使用 CSRF 令牌：** 在您的表单中包含一个 CSRF 令牌，并在服务器端验证该令牌.
        2.  **验证 Referer 头部：** 验证 Referer 头部以确保请求来自您的网站.
        3.  **使用 SameSite Cookie 属性：** 使用 SameSite Cookie 属性来限制 Cookie 的跨站点访问.

## 性能测试和基准测试

为了评估框架的性能，我们进行了一系列基准测试。

## 性能测试

本节展示了对服务器进行的初步性能基准测试结果。请注意，这些结果仅供参考，实际性能会因环境而异。

**测试环境:**

*   CPU: Intel Core i7-8700K
*   内存: 16GB DDR4
*   操作系统: Ubuntu 20.04

**测试工具:**

*   `wrk` (https://github.com/wg/wrk) - HTTP基准测试工具

**测试用例:**

*   **Hello World:** 服务器返回一个简单的 "Hello, World!" 字符串。
*   **静态网页:** 服务器返回已加载 WASM 模块的 `index.html` 页面 (更改日期:2025.03.23)。

**测试结果 (持续更新):**

| 测试用例      | 并发连接数 | 每秒请求数 (RPS) | 平均延迟 (毫秒) |
| ----------- | -------- | ------------- | ----------- |
| Hello World | 100      | 10000         | TBD         |
| Hello World | 1000     | 80000         | TBD         |
| 静态网页    | 100      | -             | -           |
| 静态网页    | 1000     | -             | -           |

*TBD: 待确定 (To Be Determined)*

**测试命令:**

```bash
wrk -t12 -c100 -d10s http://127.0.0.1:8765
```

参数说明:

*   `-t12`: 使用 12 个线程。
*   `-c100`: 保持 100 个并发连接。
*   `-d10s`: 测试持续 10 秒。

**免责声明:**

上述测试结果是在特定硬件和软件配置下获得的。 实际性能可能因硬件、网络条件、服务器配置以及其他因素而有所不同。我们强烈建议您在自己的环境中进行基准测试，以获得更准确的结果。 我们会定期更新这些测试结果，并添加更多测试用例。

## 许可证

该项目已获得 [MIT 许可证](LICENSE) 的许可。

## 联系方式

MiracleHcat@gmail.com