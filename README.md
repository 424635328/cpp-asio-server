# 服务器框架 (Boost.Asio)

一个基于 Boost.Asio 构建的轻量级、高性能的服务器框架，旨在易于扩展和定制。

## 特性

*   **高性能：** 利用 Boost.Asio 的异步、非阻塞 I/O 操作，能够高效地处理并发连接。
*   **模块化设计：** 可以轻松地使用自定义协议和请求处理程序进行扩展。
*   **多线程：** 使用线程池来处理传入的连接和处理请求。
*   **HTTP 服务器示例：** 包含一个基本的 HTTP 服务器实现，用于演示框架的功能。
*   **跨平台：** 设计为跨平台，可在 Windows 和 Linux 上运行（需要适当的构建配置）。

## 快速入门

### 前提条件

*   **Boost 库：** 异步 I/O 和多线程所必需。 使用你的系统的包管理器或 Vcpkg 安装 Boost（推荐用于 Windows）。 有关详细信息，请参阅[安装](#安装)部分。
*   **CMake：** 构建项目所必需。 从 [https://cmake.org/](https://cmake.org/) 下载并安装 CMake。
*   **MinGW（可选，用于 Windows）：** 如果在 Windows 上构建，建议使用 MinGW。

### 安装

#### 使用 Vcpkg（推荐用于 Windows）

1.  克隆 Vcpkg 仓库：

    ```bash
    git clone https://github.com/microsoft/vcpkg.git
    ```

2.  引导 Vcpkg：

    ```bash
    cd vcpkg
    .\bootstrap-vcpkg.bat   (PowerShell)
    ./bootstrap-vcpkg.sh    (Git Bash)
    ```

3.  将 Vcpkg 与你的系统集成：

    ```bash
    .\vcpkg integrate install
    ```

4.  安装 Boost：

    ```bash
    vcpkg install boost:x64-windows   # 64 位构建
    vcpkg install boost:x86-windows   # 32 位构建
    ```

#### 使用 apt（Debian/Ubuntu Linux）

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

2.  创建一个构建目录：

    ```bash
    mkdir build
    cd build
    ```

3.  使用 CMake 生成构建文件：

    *   **使用 MinGW (Windows)：**

        ```bash
        cmake .. -G "MinGW Makefiles"
        ```

    *   **使用 Visual Studio (Windows)：**

        ```bash
        cmake -B build -S . -A x64
        ```

    *   **使用 Make (Linux/macOS)：**

        ```bash
        cmake ..
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

1.  导航到构建目录。
2.  运行可执行文件：

    ```bash
    ./my_server_framework  # Linux/macOS
    .\my_server_framework.exe  # Windows
    ```

3.  打开你的 Web 浏览器，然后访问 `http://localhost:8088`。 你应该看到 "Hello, World!"。

## 配置

可以通过修改 `main.cpp` 文件来配置服务器。 你可以更改监听端口：

```c++
MyHttpServer server(io_context, 8088); // 监听 8088 端口
```

可以将打印信息全部改为英语减少执行cmake时的警告&报错

## 贡献

欢迎贡献！ 请遵循以下准则：

1.  Fork 仓库。
2.  为你添加新功能或修复错误创建一个新分支。
3.  进行更改并提交，提交信息清晰明了。
4.  提交 pull request。

## 许可证

此项目已获得 [MIT 许可证](LICENSE) 的许可。

## 联系方式

MiracleHcat@gmail.com
