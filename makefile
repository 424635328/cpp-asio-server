# 编译client.cpp文件为client.exe可执行文件
CXX = g++

# 编译选项
CXXFLAGS = -std=c++17 -Wall -g

# 头文件包含路径 (根据你的 Boost 安装路径修改)
INC = -I"E:/AAAGreat/Anaconda/Library/include" -I"E:/AAAGreat/vcpkg/installed/x64-windows/include"

# 链接库路径 (根据你的 Boost 安装路径修改)
LIB = -L"E:/AAAGreat/Anaconda/Library/lib" -L"E:/AAAGreat/vcpkg/installed/x64-windows/lib"

# 链接库
LIBS = -lboost_system -lboost_thread -lws2_32 -pthread

# 源文件
SRCS = client.cpp

# 目标文件
OBJS = $(SRCS:.cpp=.o)

# 可执行文件名
TARGET = client.exe

# 默认目标
all: $(TARGET)

# 编译规则
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIB) $(LIBS)

# 编译 .cpp 文件为 .o 文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INC) -c $< -o $@

# 清理规则
clean:
	rm -f $(OBJS) $(TARGET)