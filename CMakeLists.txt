cmake_minimum_required(VERSION 3.16)
project(HFTBenchmark LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
)

find_package(benchmark CONFIG REQUIRED)

add_executable(HFTBenchmark ${LIB_SOURCES})
target_link_libraries(
    HFTBenchmark
    PRIVATE
    benchmark::benchmark
    benchmark::benchmark_main
)
target_include_directories(HFTBenchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)
target_compile_definitions(
    HFTBenchmark
    PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>
)