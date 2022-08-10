cmake_minimum_required(VERSION 3.20)

project(Pipelining)

find_package(Threads)

find_package(pybind11 CONFIG REQUIRED)

set(CMAKE_PREFIX_PATH lib/cmake/easy_profiler)

find_package(easy_profiler REQUIRED)

set(target_sources 
        pipelining.cpp 
)

list(TRANSFORM target_sources PREPEND "src/")

add_executable(Pipelining ${target_sources})

target_compile_definitions(Pipelining PUBLIC -DBUILD_WITH_EASY_PROFILER=1)

message( ${CMAKE_CURRENT_SOURCE_DIR} )

target_include_directories(Pipelining PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Pipelining PRIVATE Threads::Threads easy_profiler)

set_property(TARGET Pipelining PROPERTY CXX_STANDARD 17)

set(target_sources_pybind 
        test.cpp 
)

list(TRANSFORM target_sources_pybind PREPEND "src/")

pybind11_add_module(module_name ${target_sources_pybind})

target_include_directories(module_name PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(module_name PUBLIC pybind11::module)

set_property(TARGET module_name PROPERTY CXX_STANDARD 17)
