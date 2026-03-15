cmake_minimum_required(VERSION 3.15)
project(unittest-pvm VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)
find_package(GTest CONFIG REQUIRED)

add_executable(unittest-pvm
        unittest.cpp
)

target_include_directories(unittest-pvm PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
)

target_link_libraries(unittest-pvm PRIVATE
        Threads::Threads
        GTest::gtest
        GTest::gtest_main
)

enable_testing()
add_test(NAME unittest-pvm COMMAND unittest-pvm)
