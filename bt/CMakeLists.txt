cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(APP_NAME bt)
set(APP_LONG_NAME "Browser Tamer")
set(APP_URL "https://www.aloneguid.uk/projects/bt/")
set(APP_GITHUB_URL "https://github.com/aloneguid/bt")
set(APP_GITHUB_RELEASES_URL "https://github.com/aloneguid/bt/releases")

find_package(nlohmann_json CONFIG REQUIRED)
find_path(P_RANAV_CSV2_INCLUDE_DIRS "csv2/mio.hpp")
find_package(tinyxml2 CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
enable_testing()

file(GLOB core_src
    "cmdline.cpp"
    "config_custom.cpp"
    "app/*.cpp"
    "app/ui/*.cpp"
    "app/pipeline/*.cpp"
    "../common/http.cpp"
    "../common/hashing.cpp"
    "../common/url.cpp"
    "../common/process.cpp"
    "../common/config/config.cpp")

if(WIN32)
file(GLOB core_src
    ${core_src}
    "../common/win32/window.cpp"
    "../common/win32/shell.cpp"
    "../common/win32/os.cpp"
    "../common/win32/reg.cpp"
    "../common/win32/uwp.cpp"
    "../common/win32/user.cpp")

    add_executable (bt WIN32 ${core_src} "bt.cpp" bt.rc)
else()
    add_executable (bt ${core_src} "bt.cpp")
endif()

file(GLOB test_src ${core_src} "tests/*.cpp")
add_executable(test ${test_src})
include(GoogleTest)
gtest_discover_tests(test)

target_link_libraries(bt
	libgrey
	nlohmann_json::nlohmann_json
    tinyxml2::tinyxml2
    unofficial::sqlite3::sqlite3
    ${LUA_LIBRARIES}
)

target_link_libraries(test PRIVATE
        GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
        libgrey
        nlohmann_json::nlohmann_json
        tinyxml2::tinyxml2
        unofficial::sqlite3::sqlite3
        ${LUA_LIBRARIES})

target_include_directories(${APP_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "../grey/grey/common"
    "../grey/grey"
    ${P_RANAV_CSV2_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR})

target_include_directories(test PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "../grey/grey/common"
        "../grey/grey"
        ${P_RANAV_CSV2_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIR})

if(WIN32)
    target_sources(bt PRIVATE dpi-aware.manifest)

    target_link_options(bt PRIVATE
            /SUBSYSTEM:WINDOWS
            /ENTRY:wmainCRTStartup
    )

    # include .pdb symbol generation
    if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC" AND CMAKE_BUILD_TYPE MATCHES "Release")
       target_compile_options(${APP_NAME} PRIVATE /Zi)

       # Tell linker to include symbol data
        set_target_properties(${APP_NAME} PROPERTIES 
            LINK_FLAGS "/INCREMENTAL:NO /DEBUG /OPT:REF /OPT:ICF"
        )

        # Set file name & location
        set_target_properties(${APP_NAME} PROPERTIES 
            COMPILE_PDB_NAME ${APP_NAME} 
            COMPILE_PDB_OUTPUT_DIR ${CMAKE_BINARY_DIR}
        )
    endif()
endif()