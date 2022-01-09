cmake_minimum_required(VERSION 3.21)
project(matching_engine)

set(CMAKE_CXX_STANDARD 20)

add_library(
        ThreadPool
        include/ThreadPool.hpp
        lib/ThreadPool.cpp
)

add_library(
        TradeDB
        include/Entry.hpp
        lib/Entry.cpp
        include/TradeDB.hpp
        lib/TradeDB.cpp
)

add_executable(matching_engine main.cpp)

target_link_libraries(matching_engine PRIVATE TradeDB ThreadPool)

enable_testing()
add_subdirectory(tests)