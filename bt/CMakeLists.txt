cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(APP_NAME bt)
set(APP_LONG_NAME "Browser Tamer")
set(APP_URL "https://www.aloneguid.uk/projects/bt/")
set(APP_GITHUB_URL "https://github.com/aloneguid/bt")
set(APP_GITHUB_RELEASES_URL "https://github.com/aloneguid/bt/releases")

find_package(fmt CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_path(P_RANAV_CSV2_INCLUDE_DIRS "csv2/mio.hpp")
find_package(tinyxml2 CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

file(GLOB core_src CONFIGURE_DEPENDS
    "cmdline.cpp"
    "app/*.cpp"
    "app/ui/*.cpp"
    "app/pipeline/*.cpp"
    "../common/*.cpp"

    "../common/win32/reg.cpp"
    "../common/win32/http.cpp"
    "../common/win32/shell.cpp"
    "../common/win32/ole32.cpp"
    "../common/win32/process.cpp"
    "../common/win32/clipboard.cpp"
    "../common/win32/sysinfo.cpp"
    "../common/win32/user.cpp"
    "../common/win32/kernel.cpp"
    "../common/win32/window.cpp"
    "../common/win32/uwp.cpp"
    "../common/win32/app.cpp"
    "../common/win32/shell_notify_icon.cpp"
    "../common/win32/popup_menu.cpp"

    "../common/config/config.cpp")
add_executable (bt WIN32 ${core_src} "bt.cpp" bt.rc)

# todo: move these to strings.h
target_compile_definitions(${APP_NAME} PRIVATE
    APP_SHORT_NAME="${APP_NAME}"
    APP_LONG_NAME="${APP_LONG_NAME}"
    APP_VERSION="${PROJECT_VERSION}"
    APP_URL="${APP_URL}"
    APP_TEST_URL="https://www.aloneguid.uk/other/bt-test-page/"
    APP_GITHUB_URL="${APP_GITHUB_URL}"
    APP_GITHUB_RELEASES_URL="${APP_GITHUB_RELEASES_URL}"
    APP_DOCS_URL="https://www.aloneguid.uk/projects/bt/"
    APP_HELP_BASE_URL="https://www.aloneguid.uk/projects/bt/"
    APP_BUYMEACOFFEE_URL="https://www.buymeacoffee.com/alonecoffee"
    APP_REG_DESCRIPTION="Redirects open URLs to a browser of your choice."
)

target_link_libraries(bt
	libgrey
	fmt::fmt-header-only
	nlohmann_json::nlohmann_json
    tinyxml2::tinyxml2
    unofficial::sqlite3::sqlite3
    ${LUA_LIBRARIES}
)
target_include_directories(${APP_NAME} PRIVATE
    "../common"
    "../grey/grey"
    ${P_RANAV_CSV2_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR})

target_sources(bt PRIVATE dpi-aware.manifest)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:wmainCRTStartup")

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