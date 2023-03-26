cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME TreeGenerator)
set(CMAKE_CXX_STANDARD 17)

project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} src/main.cpp)

# ExternalProject 관련 명령어 셋 추가
include(ExternalProject)
