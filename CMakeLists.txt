cmake_minimum_required(VERSION 3.15)
project(my_server_framework)

set(CMAKE_TOOLCHAIN_FILE "E:\\AAAGreat\\vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "Vcpkg Toolchain File" FORCE)

message(STATUS "CMAKE_TOOLCHAIN_FILE: ${CMAKE_TOOLCHAIN_FILE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 强制 CMake 重新检查 Boost 库的配置
set(Boost_USE_STATIC_LIBS OFF)   # 强制使用动态库
set(Boost_USE_MULTITHREADED ON)  # 强制使用多线程库
set(Boost_DEBUG ON)              # 启用调试信息

# Find Boost
find_package(Boost REQUIRED COMPONENTS system thread)

if(Boost_FOUND)
    message(STATUS "Boost_FOUND is TRUE")
    message(STATUS "Boost_INCLUDE_DIRS: ${Boost_INCLUDE_DIRS}")
    message(STATUS "Boost_LIBRARIES: ${Boost_LIBRARIES}")
    message(STATUS "Boost_VERSION: ${Boost_VERSION}")
    message(STATUS "Boost_THREAD_LIBRARY: ${Boost_THREAD_LIBRARY}") # 打印 Boost 线程库的路径
else()
    message(STATUS "Boost_FOUND is FALSE")
    message(FATAL_ERROR "Boost library not found. Please install it or specify its location.")
endif()

# Source files
file(GLOB_RECURSE SOURCES src/*.cpp src/*.h)

add_executable(my_server_framework ${SOURCES})

target_link_libraries(my_server_framework Boost::system Boost::thread ws2_32)