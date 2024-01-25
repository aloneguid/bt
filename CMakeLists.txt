cmake_minimum_required (VERSION 3.20)
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

# The cool "edit & continue" feature of MSVC is disabled by default in CMake.
# https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#edit-and-continue-for-cmake-projects
# enable globally: https://stackoverflow.com/a/76254117/80858
if(MSVC AND CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options("/ZI")
    add_link_options("/INCREMENTAL")
endif()

if(WIN32)
    add_subdirectory(grey)
    add_subdirectory(bt)
endif()

add_subdirectory(test)
