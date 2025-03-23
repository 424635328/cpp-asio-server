# Server Framework (Boost.Asio)                            [English](README.md) | [中文 (Chinese)](README_zh.md)

A lightweight, high-performance server framework built on Boost.Asio, designed for easy extension and customization.

## Features

*   **High Performance:** Leverages Boost.Asio's asynchronous, non-blocking I/O operations for efficient handling of concurrent connections.
*   **Modular Design:**  Easily extensible with custom protocols and request handlers.
*   **Multi-threaded:** Uses a thread pool to handle incoming connections and process requests.
*   **HTTP Server Example:** Includes a basic HTTP server implementation to demonstrate the framework's capabilities.
*   **Cross-Platform:** Designed to be cross-platform and run on Windows and Linux (requires appropriate build configuration).

## Quick Start

### Prerequisites

*   **Boost Libraries:** Required for asynchronous I/O and multi-threading. Install Boost using your system's package manager or Vcpkg (recommended for Windows). See the [Installation](#installation) section for details.
*   **CMake:** Required for building the project. Download and install CMake from [https://cmake.org/](https://cmake.org/).
*   **MinGW (Optional, for Windows):** Recommended for building on Windows.

### Installation

#### Using Vcpkg (Recommended for Windows)

1.  Clone the Vcpkg repository:

    ```bash
    git clone https://github.com/microsoft/vcpkg.git
    ```

2.  Bootstrap Vcpkg:

    ```bash
    cd vcpkg
    .\bootstrap-vcpkg.bat   (PowerShell)
    ./bootstrap-vcpkg.sh    (Git Bash)
    ```

3.  Integrate Vcpkg with your system:

    ```bash
    .\vcpkg integrate install
    ```

4.  Install Boost:

    ```bash
    vcpkg install boost:x64-windows   # 64-bit build
    vcpkg install boost:x86-windows   # 32-bit build
    ```

#### Using apt (Debian/Ubuntu Linux)

```bash
sudo apt update
sudo apt install libboost-dev libboost-system-dev libboost-thread-dev
```

#### Using brew (macOS)

```bash
brew install boost
```

### Building the Project

1.  Clone the repository:

    ```bash
    git clone https://github.com/424635328/cpp-asio-server
    cd cpp-asio-server
    ```

2.  Create a build directory:

    Modify CMakeLists.txt: set(CMAKE_TOOLCHAIN_FILE "your/path/to/vcpkg.cmake"
    CACHE STRING "Vcpkg toolchain file" FORCE)

    ```bash
    mkdir build
    cd build
    ```

3.  Generate build files using CMake:

    *   **Using MinGW (Windows):**

        ```bash
        cmake .. -G "MinGW Makefiles"
        ```

    *   **Using Visual Studio (Windows):**

        ```bash
        cmake -B build -S . -A x64
        ```

    *   **Using Make (Linux/macOS):**

        ```bash
        cmake ..
        ```

4.  Build the project:

    *   **Using MinGW:**

        ```bash
        mingw32-make
        ```

    *   **Using Visual Studio:**

        ```bash
        cmake --build . --config Release
        ```

    *   **Using Make:**

        ```bash
        make
        ```

### Running the Server

1.  Navigate to the build directory `build/release/`.
2.  Run the executable:

    ```bash
    ./my_server_framework  # Linux/macOS
    ./my_server_framework.exe  # Windows
    ```

3.  Open your web browser and go to `http://127.0.0.1:8765`. You should see "Hello, World!".
4.  We can check if the server is running
    ```bash
    curl http://127.0.0.1:8765 # Linux/macOS
    curl http://127.0.0.1:8765 -o /dev/null  # Windows
    ```
    Or
    ```bash
    netstat -ano | findstr :8765 # Windows
    netstat -tulnp | grep :8765 # Linux/macOS
    ```

## Configuration

The server can be configured by modifying the `main.cpp` file. You can change the listening port:

```c++
MyHttpServer server(io_context, 8765);
```

## Contributing

Contributions are welcome! Please follow these guidelines:

1.  Fork the repository.
2.  Create a new branch for your feature or bug fix.
3.  Make your changes and commit them with clear and concise commit messages.
4.  Submit a pull request.

## License

This project is licensed under the [MIT License](LICENSE).

## Contact

MiracleHcat@gmail.com