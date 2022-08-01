cmake_minimum_required(VERSION 3.20)

project(Pipelining)

find_package(Threads)

set(CMAKE_PREFIX_PATH lib/cmake/easy_profiler)

find_package(easy_profiler REQUIRED)

add_executable(Pipelining pipelining.cpp pipeline_stages.cpp)

target_compile_definitions(Pipelining PUBLIC -DBUILD_WITH_EASY_PROFILER=1)

target_link_libraries(Pipelining PRIVATE Threads::Threads easy_profiler)