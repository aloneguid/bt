cmake_minimum_required (VERSION 3.31)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
project ("BrowserTamer" VERSION 6.3.0 LANGUAGES CXX OBJCXX)
else()
project ("BrowserTamer" VERSION 6.3.0)
endif()

# Application metadata - shared between main build and packaging
set(APP_NAME bt)
set(APP_LONG_NAME "Browser Tamer")
set(APP_URL "https://www.aloneguid.uk/projects/bt/")
set(APP_GITHUB_URL "https://github.com/aloneguid/bt")
set(APP_GITHUB_RELEASES_URL "https://github.com/aloneguid/bt/releases")

# declare Unicode pair: UNICODE affects win32 API, _UNICODE affects crt entry point
add_compile_definitions(UNICODE _UNICODE)

function (pre_build)
    set(RELEASE_NOTES_FILE "${PROJECT_SOURCE_DIR}/docs/release-notes.md")
    message(STATUS "reading from ${RELEASE_NOTES_FILE}")

    # version
    file(STRINGS "${RELEASE_NOTES_FILE}" _first_line LIMIT_COUNT 1)
    string(REGEX REPLACE "[ #]" "" RELEASE_VERSION "${_first_line}")
    message(STATUS "version: ${RELEASE_VERSION}")

    # version variants
    set(RELEASE_VERSION_FULL "${RELEASE_VERSION}.0")
    string(REPLACE "." "," RELEASE_VERSION_COMMAS "${RELEASE_VERSION_FULL}")
    message(STATUS "versions: [${RELEASE_VERSION}], [${RELEASE_VERSION_FULL}], [${RELEASE_VERSION_COMMAS}]")

    # .rc file
    file(GLOB _rc_files "${PROJECT_SOURCE_DIR}/bt/*.rc")
    foreach(_rc IN LISTS _rc_files)
        file(READ "${_rc}" _content)
        set(_original "${_content}")
        # \d,\s*\d+,\s*\d+,\s*\d+   ->  v1
        string(REGEX REPLACE
                "[0-9]+,[ \t]*[0-9]+,[ \t]*[0-9]+,[ \t]*[0-9]+"
                "${RELEASE_VERSION_COMMAS}" _content "${_content}")

        # \d\.\d+\.\d+\.\d+         ->  v2
        string(REGEX REPLACE
                "[0-9]+\\.[0-9]+\\.[0-9]+\\.[0-9]+"
                "${RELEASE_VERSION_FULL}" _content "${_content}")

        # only write if something changed (avoids needless rebuilds)
        if(NOT _content STREQUAL _original)
            file(WRITE "${_rc}" "${_content}")
            message(STATUS "Updated version in ${_rc}")
        endif()
    endforeach()

    # globals.h
    set(_globals "${PROJECT_SOURCE_DIR}/bt/globals.h")
    if(EXISTS "${_globals}")
        message(STATUS "Updating version in: ${_globals}")

        file(READ "${_globals}" _content)
        set(_original "${_content}")

        # #define APP_VERSION ".*"  ->  #define APP_VERSION "<version>"
        string(REGEX REPLACE
                "#define APP_VERSION \"[^\"\n]*\""
                "#define APP_VERSION \"${RELEASE_VERSION}\""
                _content "${_content}")

        if(NOT _content STREQUAL _original)
            file(WRITE "${_globals}" "${_content}")
            message(STATUS "Updated: ${_globals}")
        endif()
    endif()

    # this file
    set(_cmakelists "${PROJECT_SOURCE_DIR}/bt/CMakeLists.txt")

    if(EXISTS "${_cmakelists}")
        message(STATUS "Updating version in: ${_cmakelists}")

        file(READ "${_cmakelists}" _content)
        set(_original "${_content}")

        # set(APP_VERSION "...")  ->  set(APP_VERSION "<version>")
        set(VAR_NAME "${project}_APP_VERSION")
        string(TOUPPER "${VAR_NAME}" VAR_NAME)
        string(REGEX REPLACE
                "set\\(${VAR_NAME} \"[^\"]*\"\\)"
                "set(${VAR_NAME} \"${RELEASE_VERSION}\")"
                _content "${_content}")

        if(NOT _content STREQUAL _original)
            file(WRITE "${_cmakelists}" "${_content}")
            message(STATUS "Updated: ${_cmakelists}")
        endif()
    endif()

endfunction()

pre_build()

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
    if(MSVC)
        set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "EditAndContinue" CACHE STRING "" FORCE)
        add_link_options("/INCREMENTAL")
    endif()

    if(MSVC AND CMAKE_BUILD_TYPE MATCHES Release)
        add_link_options("/OPT:REF")
    endif()
endif()

add_subdirectory(grey/grey)
add_subdirectory(grey/demo/desktop)
add_subdirectory(bt)
