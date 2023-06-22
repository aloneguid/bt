cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("btu" VERSION 3.1.0)

add_compile_definitions(UNICODE) # required for WIN32 API to use correct versions

#set_property(TARGET bt PROPERTY
#  MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # turns on Hot Reload
	set(CMAKE_CXX_FLAGS_DEBUG "/MDd /ZI /Ob0 /Od /RTC1")

    # optimisations (prefer size)
    set(CMAKE_CXX_FLAGS "/O1s")
    set(CMAKE_EXE_LINKER_FLAGS "/OPT:REF")
endif()

add_subdirectory(grey)
add_subdirectory(bt)
