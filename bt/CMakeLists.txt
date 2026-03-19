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
    "app/pipeline/*.cpp")
add_executable (bt WIN32 ${core_src} "bt.cpp" bt.rc)

target_link_libraries(bt
    libcommon
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