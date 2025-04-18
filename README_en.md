# Server Framework (Boost.Asio)

[Chinese](README.md) | [English](README_en.md) | [中文简化版](README_lite.md)

A lightweight, high-performance server framework built on Boost.Asio, designed to offer easy extensibility and customization.

## Features

*   **High Performance:** Leverages Boost.Asio's asynchronous, non-blocking I/O operations for efficient handling of concurrent connections.
*   **Modular Design:** Easily extendable with custom protocols and request handlers.
*   **Multi-threading:** Uses a thread pool to manage incoming connections and request processing, maximizing throughput.
*   **HTTP Server Example:** Includes a basic HTTP server implementation demonstrating the framework's capabilities out-of-the-box.
*   **Cross-Platform:** Designed for cross-platform compatibility, supporting Windows and Linux (requires appropriate build configurations).
*   **Configurable Thread Pool:** Adjust the thread pool size based on hardware resources and anticipated workload.
*   **Error Handling and Logging:** Provides consistent error handling and logging mechanisms throughout the framework.

## Architectural Overview

The core components of the framework include:

*   **Server:** Listens on a specified port, accepts new connections, and dispatches them to worker threads.
*   **Connection:** Represents a client connection, responsible for reading data, writing data, and handling protocol-specific details.
*   **IOContext:** The heart of Boost.Asio, managing asynchronous I/O operations.  Think of it as the engine that drives all the asynchronous tasks.
*   **ThreadPool:** A pool of threads used to execute the connection's processing logic, improving concurrency and responsiveness.  Threads are reused for different connections, reducing overhead.
*   **RequestHandler:** An abstract class (interface) that defines how client requests are processed. You implement concrete `RequestHandler` classes to handle specific request types or business logic.
*   **ProtocolHandler:** Handles the specifics of a particular protocol (e.g., HTTP). Responsible for parsing incoming data according to the protocol's rules and formatting outgoing data.

These components work together to provide an efficient and scalable server framework.  The `Server` accepts incoming connections, creates a `Connection` object for each, and then uses the `ThreadPool` to assign a thread to handle the connection's processing. The `Connection` object uses the `ProtocolHandler` to manage protocol-related details (like HTTP headers) and passes the processed request to the `RequestHandler` for business logic execution.

## Getting Started

### Prerequisites

*   **Boost Library:** Required for asynchronous I/O and multi-threading. Install using your system's package manager or Vcpkg (recommended for Windows). See the [Installation](#installation) section for detailed instructions.
*   **CMake:** Needed to build the project. Download and install CMake from [https://cmake.org/](https://cmake.org/).
*   **MinGW (Optional, Windows):** Recommended for building on Windows using the MinGW toolchain.

### Installation

#### Using Vcpkg (Recommended for Windows)

Vcpkg is a package manager for C++ libraries on Windows, simplifying the installation process.

1.  Clone the Vcpkg repository:

    ```bash
    git clone https://github.com/microsoft/vcpkg.git
    ```

2.  Bootstrap Vcpkg:  This prepares Vcpkg for use on your system.

    ```bash
    cd vcpkg
    ./bootstrap-vcpkg.bat   # PowerShell
    ./bootstrap-vcpkg.sh    # Git Bash
    ```

3.  Integrate Vcpkg with your system:  This sets up Vcpkg to be used by CMake.

    ```bash
    ./vcpkg integrate install
    ```

4.  Install Boost:  This downloads and builds the Boost libraries.  Choose the correct architecture (x64 for 64-bit builds, x86 for 32-bit).

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

2.  Create a build directory:  It's good practice to build in a separate directory to keep your source code clean.

    **Important:** Before generating build files, you **must** tell CMake where to find Vcpkg. Modify the `CMakeLists.txt` file to specify the Vcpkg toolchain file path:

    ```cmake
    set(CMAKE_TOOLCHAIN_FILE "<path_to_vcpkg>/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg Toolchain File" FORCE)
    ```
    Replace `<path_to_vcpkg>` with the actual path to your Vcpkg installation.

    ```bash
    mkdir build
    cd build
    ```

3.  Generate build files using CMake:  The command you use depends on your build environment.

    *   **Using MinGW (Windows):**

        ```bash
        cmake .. -G "MinGW Makefiles"
        ```

    *   **Using Visual Studio (Windows):**

        ```bash
        cmake -B . -S .. -A x64 -DCMAKE_BUILD_TYPE=Release  # Explicitly specify architecture and build type
        ```

    *   **Using Make (Linux/macOS):**

        ```bash
        cmake .. -DCMAKE_BUILD_TYPE=Release  # Explicitly specify build type
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

1.  Navigate to the build directory (typically `build/` or `build/Release/`, depending on your build system).
2.  Run the executable:

    ```bash
    ./my_server_framework  # Linux/macOS
    ./my_server_framework.exe  # Windows
    ```

3.  Open your web browser and visit `http://127.0.0.1:8765`. You should see the expected response (e.g., "Hello, World!" or the contact form).
4.  Verify the server is running:

    ```bash
    curl http://127.0.0.1:8765  # Linux/macOS
    curl http://127.0.0.1:8765 -o nul # Windows
    ```

    Alternatively, use `netstat`:

    ```bash
    netstat -ano | findstr :8765  # Windows
    netstat -tulnp | grep :8765 # Linux/macOS
    ```

## HTTP Server Example in Detail

The framework includes a simple HTTP server example demonstrating how to handle HTTP requests.

**Key Components:**

*   `MyHttpServer`: The server class, responsible for listening for connections, accepting them, and managing the maximum number of concurrent connections.
*   `HttpRequestHandler`: Handles the logic for processing HTTP requests and generating appropriate responses.  This is where you define your server's endpoints and behavior.
*   `HttpSession`: Manages individual client sessions, handling reading requests, sending responses, and potentially keeping connections alive (HTTP keep-alive).

**Code Example:**

```c++
#include "server.h"
#include "http_request_handler.h"
#include "asio_context.h"

int main() {
  // Determine the number of hardware threads available.
  size_t num_threads = std::thread::hardware_concurrency();

  // Create an AsioContext with a thread pool of the determined size. This manages the execution of asynchronous tasks.
  AsioContext io_context(num_threads);

  // Create the HTTP server, specifying the io_context, port to listen on (8765), and maximum number of connections (1000).
  MyHttpServer server(io_context, 8765, 1000);

  // Start the io_context, which begins processing asynchronous tasks.
  io_context.run();

  return 0;
}
```

**Request Handling:**

The `HttpRequestHandler` is responsible for parsing HTTP requests and generating responses based on their content. This includes handling static file requests (serving HTML, CSS, JavaScript, images) and processing the example `/contact` form submission.

**Extending the HTTP Server:**

You can easily extend the HTTP server by modifying the `HttpRequestHandler` to add new routes and processing logic.  Refer to the `HttpRequestHandler::handle_request` function for examples of how to add new endpoints and define their behavior.

## Configuration

The server can be configured either via command-line options or by modifying the `main.cpp` file.  You can change the listening port, maximum connections, and other parameters.

```bash
./my_server_framework --port 9000 --max_connections 2000
```

```c++
// main.cpp (Example)
int main(int argc, char* argv[]) {
    // ... (program_options setup using Boost.Program_options)
    short port = vm["port"].as<short>();
    size_t max_connections = vm["max_connections"].as<size_t>();
    MyHttpServer server(*io_context, port, max_connections);
    // ...
}
```

**Thread Pool Configuration:**

The size of the thread pool is configured within the `AsioContext` constructor. Generally, set it to equal the number of hardware threads available for optimal performance.

```c++
size_t num_threads = std::thread::hardware_concurrency();
AsioContext io_context(num_threads);
```

**Error Handling and Logging:**

The framework leverages Boost.Asio's `boost::system::error_code` for consistent error handling in asynchronous operations. Check the `error_code` object after each asynchronous operation to determine if the operation was successful. Basic logging is provided using `std::cout` and `std::cerr`, with all console output protected by a mutex for thread safety.

**Note:** Ensure all console output (including logging messages) is in English to minimize potential warnings and errors during CMake execution and to maintain consistency.

## Extending Protocol Support

The framework allows you to add support for custom protocols by implementing the `ProtocolHandler` interface.

1.  **Define a Protocol Handler Class:** Create a class that inherits from `ProtocolHandler`.  This class will contain the logic for parsing and generating protocol-specific data.

    ```c++
    class MyProtocolHandler : public ProtocolHandler {
    public:
        // Implement the protocol parsing logic.  Extract the request from the buffer, return the request string and the number of bytes consumed.
        std::pair<std::string, size_t>  parseRequest(boost::asio::streambuf& buffer) override {
            // ... (Protocol parsing logic) ...
        }

        // Convert the response string into a sequence of buffers that can be sent over the socket.
        std::vector<boost::asio::const_buffer>  createResponse(const std::string& response) override {
            // ... (Response formatting logic) ...
        }
    };
    ```

2.  **Register the Protocol Handler:** In the `Connection` class, determine the protocol being used (e.g., by inspecting the initial bytes of the connection) and select the appropriate `ProtocolHandler` implementation.

    ```c++
    // Inside the Connection class:
    void Connection::start() {
      // Determine the protocol (e.g., by reading the initial bytes)
      if (isMyProtocol()) {
        protocol_handler_ = std::make_shared<MyProtocolHandler>();
      } else {
        protocol_handler_ = std::make_shared<HttpProtocolHandler>();
      }
      doRead();  // Start reading data from the socket using the selected protocol handler.
    }
    ```

## More Code Examples

*   **Simple Echo Server:**

    ```c++
    class EchoRequestHandler : public RequestHandler {
    public:
      HttpResponse handle_request(const HttpRequest& request) override {
        HttpResponse response;
        response.status_code = 200;
        response.body = request.body; // Echo back the request body.  Whatever the client sends, the server sends back.
        response.headers["Content-Type"] = "text/plain";
        return response;
      }
    };
    ```

*   **Static File Server:**

    The framework's `HttpRequestHandler` already includes static file serving functionality. Place your files in the `web/` directory, and they will be accessible via HTTP. By default, accessing the root path (`/`) will serve the `web/index.html` file.

## Code Style Conventions

To maintain consistency and readability within the codebase, please adhere to the following code style conventions:

*   Use 4 spaces for indentation.
*   Use PascalCase (also known as upper CamelCase) for class and function names (e.g., `MyClass`, `CalculateValue`).
*   Use snake_case for variable names (e.g., `my_variable`, `user_name`).
*   Add appropriate comments to explain the purpose and functionality of your code.  Focus on *why* the code does what it does, not just *what* it does.
*   Write comments and documentation in English.

## Contributing

Contributions are welcome! Please follow these guidelines:

1.  Fork the repository.
2.  Create a new branch for your feature or bug fix.  Give the branch a descriptive name.
    ```bash
    git checkout -b feature/my-new-feature
    ```
3.  Implement your changes and commit them with clear, concise, and informative commit messages.

    *   Commit messages should summarize the changes made in the commit.
    *   If your changes fix a bug, include the bug number in the commit message.
    *   If your changes add a new feature, provide a brief code example demonstrating the feature's usage.

    ```bash
    git commit -m "Add my new feature: Implemented X, Y, and Z"
    ```

4.  Before submitting a pull request, ensure your code adheres to the code style conventions.
5.  Run all unit tests (if applicable) to verify that your changes have not introduced regressions.
6.  Submit a pull request.

    *   In the pull request description, provide a detailed explanation of the changes you've made.
    *   If your changes fix a bug, include the bug number in the pull request description.
    *   If your changes add a new feature, provide a brief code example demonstrating the feature's usage in the pull request description.  This makes it easier for reviewers to understand your changes.

7.  Code Review: Other contributors will review your code, provide feedback, and suggest improvements. Be prepared to address any feedback you receive.
8.  Testing: Your code will be automatically tested to ensure that it does not break existing functionality.
9.  Merge: Once your code has passed code review and testing, it will be merged into the main branch.

## FAQ (Frequently Asked Questions)

*   **Q: How do I handle a high volume of concurrent connections?**

    *   A: The framework uses Boost.Asio's asynchronous I/O and a thread pool to efficiently manage high concurrency.  To increase capacity, you can adjust the thread pool size and the maximum number of allowed connections. The key is to avoid blocking operations in your request handlers.

*   **Q: How do I add support for a custom protocol?**

    *   A: Implement the `ProtocolHandler` interface and register it in the `Connection` class. See the [Extending Protocol Support](#extending-protocol-support) section for details.

*   **Q: How do I perform performance testing?**

    *   A: Use tools like `ab` (Apache Benchmark) or `wrk` to benchmark the server. See the [Performance Testing and Benchmarking](#performance-testing-and-benchmarking) section.

*   **Q: How do I build the project on Windows using Visual Studio?**

    *   A: Ensure that you have Visual Studio installed and that you specify the Visual Studio generator during CMake configuration (e.g., `cmake -B . -S .. -A x64 -DCMAKE_BUILD_TYPE=Release`). After generating the build files, you can open the generated solution file (`.sln`) in Visual Studio and build the project.

## Performance Testing and Benchmarking

This section presents preliminary performance benchmark results for the server.  Please note that these results are for informational purposes only, and actual performance may vary depending on the environment.

**Test Environment:**

*   CPU: Intel Core i7-8700K
*   Memory: 16GB DDR4
*   Operating System: Ubuntu 20.04

**Test Tool:**

*   `wrk` (https://github.com/wg/wrk) - HTTP benchmarking tool

**Test Cases:**

*   **Hello World:** The server returns a simple "Hello, World!" string.
*   **Static Page:** The server returns an `index.html` page with a loaded WASM module (date 2025.03.23 triggers this).

**Test Results (Continuously Updated):**

| Test Case   | Concurrent Connections | Requests per Second (RPS) | Average Latency (ms) |
| ---------   | -------------------- | ----------------------- | -------------------- |
| Hello World | 100                  | 10000                   | TBD                  |
| Hello World | 1000                 | 80000                   | TBD                  |
| Static Page | 100                  | -                       | -                    |
| Static Page | 1000                 | -                       | -                    |

*TBD: To Be Determined*

**Test Command:**

```bash
wrk -t12 -c100 -d10s http://127.0.0.1:8765
```

Parameter Explanations:

*   `-t12`: Use 12 threads.
*   `-c100`: Keep 100 concurrent connections.
*   `-d10s`: Test duration is 10 seconds.

**Disclaimer:**

The test results above were obtained under specific hardware and software configurations. Actual performance may vary due to differences in hardware, network conditions, server configuration, and other factors. We strongly recommend conducting benchmarks in your own environment to obtain more accurate results. We will periodically update these test results and add more test cases.

## License

This project is licensed under the [MIT License](LICENSE).

## Contact

MiracleHcat@gmail.com