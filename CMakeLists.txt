cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_NAME autotiler)
project(${PROJECT_NAME} CXX)

if (NOT CURRENT_CPM_VERSION)
    include(cmake/get_cpm.cmake)
endif()

add_executable(${PROJECT_NAME} WIN32 src/main.cpp)

CPMAddPackage("gh:dankmeme01/wsx#main")
target_link_libraries(${PROJECT_NAME} PRIVATE wsx)
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE ON)