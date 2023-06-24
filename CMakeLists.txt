cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLOBAL_VERSION "$ENV{VERSION}")
if(NOT GLOBAL_VERSION)
    set(GLOBAL_VERSION "0.0.0")
endif()

project ("btu" VERSION ${GLOBAL_VERSION})

add_compile_definitions(UNICODE) # required for WIN32 API to use correct versions

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

add_subdirectory(grey)
add_subdirectory(bt)
