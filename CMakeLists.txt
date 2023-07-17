cmake_minimum_required (VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(GLOBAL_VERSION "$ENV{VERSION}")
if(NOT GLOBAL_VERSION)
    set(GLOBAL_VERSION "0.0.0")
endif()

project ("btu" VERSION ${GLOBAL_VERSION})

add_compile_definitions(UNICODE) # required for WIN32 API to use correct versions

# statically link MSVC runtime in thrift to being fully self-contained.
# this increases .exe size but guarantees working on all windows versions
# see https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(WIN32)
    add_subdirectory(grey)
    add_subdirectory(bt)
endif()
add_subdirectory(test)
