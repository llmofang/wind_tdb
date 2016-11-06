cmake_minimum_required(VERSION 3.2)
project(TDBSaver)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)

LINK_DIRECTORIES(lib)
## add_library(TDBAPI)
add_executable(TDBSaver ${SOURCE_FILES})
target_link_libraries (TDBSaver pthread TDBAPI)