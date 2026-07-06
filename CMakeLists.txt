cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project ("BrowserTamer")

add_compile_definitions(UNICODE)    # required for WIN32 API to use correct versions
if(WIN32)
    add_compile_definitions(NOMINMAX)   # Windows SDK overrides those
    #add_compile_definitions(WIN32_LEAN_AND_MEAN)

    # statically link MSVC runtime in thrift to being fully self-contained.
    # this increases .exe size but guarantees working on all windows versions
    # see https://cmake.org/cmake/help/latest/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
    #set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

    # Conditionally set CMAKE_MSVC_RUNTIME_LIBRARY based on VCPKG_TRIPLET
    # Retrieve the VCPKG_TRIPLET environment variable
    string(TOLOWER "$ENV{VCPKG_TRIPLET}" VCPKG_TRIPLET_LOWER)
    if(VCPKG_TRIPLET_LOWER STREQUAL "x64-windows")
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()

    # The cool "edit & continue" feature of MSVC is disabled by default in CMake.
    # https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=msvc-170#edit-and-continue-for-cmake-projects
    # enable globally: https://stackoverflow.com/a/76254117/80858
    if(MSVC AND CMAKE_BUILD_TYPE MATCHES Debug)
        add_compile_options("/ZI")
        add_link_options("/INCREMENTAL")
    endif()

    if(MSVC AND CMAKE_BUILD_TYPE MATCHES Release)
        add_link_options("/OPT:REF")
    endif()
endif()

add_subdirectory(grey/grey)
add_subdirectory(grey/demo/desktop)
add_subdirectory(bt)

# CPack configuration
set(CPACK_PACKAGE_NAME "bt")
set(CPACK_PACKAGE_VENDOR "aloneguid")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Browser Tamer helps you choose the right browser for each URL.")

if(NOT CPACK_PACKAGE_VERSION)
    if(DEFINED ENV{VERSION})
        set(CPACK_PACKAGE_VERSION $ENV{VERSION})
    else()
        set(CPACK_PACKAGE_VERSION "1.0.0")
    endif()
endif()
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "aloneguid <aloneguid@gmail.com>")
set(CPACK_DEBIAN_PACKAGE_SECTION "net")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.aloneguid.uk/projects/bt/")
set(CPACK_GENERATOR "DEB")

# Act as a web browser - this is mostly handled by the .desktop file,
# but we ensure the package is marked correctly.
set(CPACK_DEBIAN_PACKAGE_DEPENDS "xdg-utils") # useful for URL handling
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_SOURCE_DIR}/resources/debian/postinst;${CMAKE_SOURCE_DIR}/resources/debian/prerm")

include(CPack)
