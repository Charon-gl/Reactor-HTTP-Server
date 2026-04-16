cmake_minimum_required(VERSION 3.15.1)
project(Reactor_Server)

#服务器程序
#查找源文件
file(GLOB SRC ${PROJECT_SOURCE_DIR}/src/*.cpp)
add_executable(Server ${SRC})
#头文件路径
set(HEADPATH ${PROJECT_SOURCE_DIR}/include)
target_include_directories(Server PRIVATE ${HEADPATH})

#客户端程序
#查找源文件
file(GLOB TEST_SRC ${PROJECT_SOURCE_DIR}/test/*.cpp)
add_executable(Client ${TEST_SRC})
target_include_directories(Client PRIVATE ${PROJECT_SOURCE_DIR}/test)