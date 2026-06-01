cmake_minimum_required(VERSION 3.15.1)
project(Reactor_Server)

#服务器程序
#查找源文件
file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(Server ${SRC})

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -fno-omit-frame-pointer")
set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

#头文件路径
set(HEADPATH ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Server PRIVATE ${HEADPATH})