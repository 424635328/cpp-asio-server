# Server Framework (Boost.Asio)                            [English](README.md) | [中文 (Chinese)](README_zh.md) | [中文简化版](README_lite.md)


一个基于 Boost.Asio 构建的轻量级、高性能服务器框架，旨在提供易于扩展和定制的能力。

## 特性

*   **高性能：** 利用 Boost.Asio 的异步非阻塞 I/O 操作，高效处理并发连接。
*   **模块化设计：** 可轻松使用自定义协议和请求处理程序进行扩展。
*   **多线程：** 使用线程池管理传入连接和请求处理。
*   **HTTP 服务器示例：** 包含一个基本的 HTTP 服务器实现，演示框架的功能。
*   **跨平台：** 设计为跨平台兼容，支持 Windows 和 Linux (需要适当的构建配置)。

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
    ./bootstrap-vcpkg.bat   (PowerShell)
    ./bootstrap-vcpkg.sh    (Git Bash)
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
        cmake -B . -S .. -A x64 -DCMAKE_BUILD_TYPE=Release # 明确指定构建类型
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

3.  打开你的 Web 浏览器，访问 `http://127.0.0.1:8765`。 你应该能看到 "Hello, World!"。
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

## 配置

可以通过修改 `main.cpp` 文件来配置服务器。 例如，您可以更改监听端口：

```c++
MyHttpServer server(io_context, 8765);
```

**注意：** 确保所有打印输出均为英文，以最大程度地减少 CMake 执行期间的警告和错误，并保持日志记录的一致性。

## 贡献

欢迎贡献！ 请遵循以下准则：

1.  Fork 仓库。
2.  为你添加新功能或修复错误创建一个新分支。
3.  实现你的更改并提交，提交信息清晰、简洁且信息丰富。
4.  提交 pull request。

## 许可证

此项目已获得 [MIT 许可证](LICENSE) 的许可。

## 联系方式

MiracleHcat@gmail.com