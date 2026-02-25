cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME autotiler)

project(${PROJECT_NAME} CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME} WIN32 src/main.cpp src/easywsclient.cpp)
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE OFF)
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra)