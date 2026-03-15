cmake_minimum_required(VERSION 3.15)
project(benchmark-pvm VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(benchmark CONFIG REQUIRED)

add_executable(benchmark-pvm
        benchmark.cpp
)

target_include_directories(benchmark-pvm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(benchmark-pvm PRIVATE
        Threads::Threads
        benchmark::benchmark
        benchmark::benchmark_main
)

enable_testing()
add_test(NAME benchmark-pvm COMMAND benchmark-pvm)
