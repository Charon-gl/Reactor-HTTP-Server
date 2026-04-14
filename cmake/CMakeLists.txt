cmake_minimum_required(3.15.1)
project(Reactor_Server)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/src *.cpp)
include_directories(${PROJECT_SOURCE_DIR}/include)
add_executable(Server ${SRC})
