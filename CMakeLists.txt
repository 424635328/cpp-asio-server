cmake_minimum_required(VERSION 3.15)
# 项目名称
project(my_server_framework)

# 设置 Vcpkg 工具链文件路径 (强制缓存，覆盖之前设置)
set(CMAKE_TOOLCHAIN_FILE "E:\\AAAGreat\\vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg 工具链文件" FORCE)

# 打印 Vcpkg 工具链文件路径 (用于调试)
message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 强制 CMake 重新检查 Boost 库的配置
set(Boost_USE_STATIC_LIBS OFF)   # 强制使用动态库
set(Boost_USE_MULTITHREADED ON)  # 强制使用多线程库
set(Boost_DEBUG ON)              # 启用调试信息

# 查找 Boost 库，system 和 thread 组件是必需的
find_package(Boost REQUIRED COMPONENTS system thread)

# 检查是否找到 Boost 库
if(Boost_FOUND)
    # 打印 Boost 库的信息 (用于调试)
    message(STATUS "Boost_FOUND is TRUE")
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}") # Boost 头文件目录
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")   # Boost 库文件路径
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")     # Boost 版本号
    message(STATUS "Boost_THREAD_LIBRARY: ${Boost_THREAD_LIBRARY}") # Boost 线程库的路径
else()
    # 如果没有找到 Boost 库，则报错并停止编译
    message(STATUS "Boost_FOUND is FALSE")
    message(FATAL_ERROR "Boost 库未找到。请安装 Boost 或指定 Boost 的安装位置。")
endif()

# 查找源文件 (递归查找 src 目录下的所有 .cpp 和 .h 文件)
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

# 添加可执行文件
add_executable(my_server_framework ${SOURCES})

# 链接 Boost 库 (system 和 thread 组件) 和 ws2_32 库 (Windows socket 库)
target_link_libraries(my_server_framework Boost::system Boost::thread ws2_32)