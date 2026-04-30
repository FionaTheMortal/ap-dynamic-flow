cmake_minimum_required(VERSION 3.10)

project(APDynamicFlow)

set(CMAKE_CXX_STANDARD 17)

add_executable(
    ${PROJECT_NAME}
    src/main.cpp
    src/ap_dynamic_flow_parser.cpp
)

target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE
    ${CMAKE_SOURCE_DIR}/extern
)
