cmake_minimum_required(VERSION 3.20)
project(profiler_timeline_api)

set(CMAKE_CXX_STANDARD 17)

add_executable(test_event_generator
        include/debug_output.hpp
        include/utility.hpp
        include/events.hpp
        include/etw_provider_api.hpp
        tests/test.cpp)

add_library(profiler_timeline_api SHARED
        include/debug_output.hpp
        include/utility.hpp
        include/events.hpp
        include/etw_provider_api.hpp
        api/exports.cpp)
