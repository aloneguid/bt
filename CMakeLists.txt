cmake_minimum_required (VERSION 3.31)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)

set(PROJECT_SLUG "bt")

project ("Browser Tamer"
        VERSION 6.3.2
        HOMEPAGE_URL "https://www.aloneguid.uk/projects/bt/"
        LANGUAGES CXX)

if(APPLE)
    enable_language(OBJCXX)
endif()

# declare Unicode pair: UNICODE affects win32 API, _UNICODE affects crt entry point
add_compile_definitions(UNICODE _UNICODE)

if(WIN32)
    add_compile_definitions(NOMINMAX)   # Windows SDK overrides those
endif()

string(TIMESTAMP CURRENT_YEAR "%Y")
configure_file(globals.h.in ${CMAKE_CURRENT_BINARY_DIR}/globals.h @ONLY)
configure_file(bt.rc.in "${CMAKE_CURRENT_SOURCE_DIR}/bt.rc" @ONLY)

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

find_package(nlohmann_json CONFIG REQUIRED)
find_path(P_RANAV_CSV2_INCLUDE_DIRS "csv2/mio.hpp")
find_package(tinyxml2 CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)
if(NOT WIN32)
    find_package(CURL REQUIRED)
endif()

include(GNUInstallDirs)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
enable_testing()

# add "grey" from github
FetchContent_Declare(grey
    #GIT_REPOSITORY git@github.com:aloneguid/grey.git
    GIT_REPOSITORY https://github.com/aloneguid/grey.git
    GIT_TAG        202610
    GIT_SHALLOW    TRUE
    SOURCE_SUBDIR  grey
)

FetchContent_MakeAvailable(grey)

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

file(GLOB core_src
        "res.cpp"
        "globals.cpp"
        "cmdline.cpp"
        "state.cpp"
        "app/*.cpp"
        "app/ui/*.cpp"
        "app/pipeline/*.cpp"
        "${grey_SOURCE_DIR}/grey/common/fss.cpp"
        "${grey_SOURCE_DIR}/grey/common/clipboard.cpp"
        "${grey_SOURCE_DIR}/grey/common/keyboard.cpp"
        "${grey_SOURCE_DIR}/grey/common/datetime.cpp"
        "${grey_SOURCE_DIR}/grey/common/process.cpp")

if(WIN32)
    file(GLOB core_src
            ${core_src}
            "${grey_SOURCE_DIR}/grey/common/win32/window.cpp"
            "${grey_SOURCE_DIR}/grey/common/win32/shell.cpp"
            "${grey_SOURCE_DIR}/grey/common/win32/os.cpp"
            "${grey_SOURCE_DIR}/grey/common/win32/reg.cpp"
            "${grey_SOURCE_DIR}/grey/common/win32/user.cpp")

    add_executable (${PROJECT_SLUG} WIN32 ${core_src} "bt.cpp" bt.rc)
else()
    add_executable (${PROJECT_SLUG} ${core_src} "bt.cpp")
endif()

if(APPLE)
    set(MACOSX_ICON "${CMAKE_SOURCE_DIR}/resources/macos/BrowserTamer.icns")
    set_source_files_properties(${MACOSX_ICON}
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    target_sources(${PROJECT_SLUG} PRIVATE ${MACOSX_ICON})

    set_target_properties(${PROJECT_SLUG} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
            MACOSX_BUNDLE_GUI_IDENTIFIER "uk.co.aloneguid.browsertamer"
            MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME}"
            MACOSX_BUNDLE_ICON_FILE "BrowserTamer.icns"
            MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
            MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
            MACOSX_BUNDLE_COPYRIGHT "Copyright © aloneguid")
endif()

file(GLOB test_src
        ${core_src}
        "tests/*.cpp"
        app/pipeline/clearurls.h
        app/pipeline/clearurls.cpp
)
add_executable(test1 ${test_src})
include(GoogleTest)
gtest_discover_tests(test1)

set(COMMON_LIBS
        libgrey
        nlohmann_json::nlohmann_json
        tinyxml2::tinyxml2
        unofficial::sqlite3::sqlite3
        ${LUA_LIBRARIES})

if(NOT WIN32)
    list(APPEND COMMON_LIBS CURL::libcurl)
endif()

set(COMMON_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${P_RANAV_CSV2_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR})

# ClearURLs database deployed next to the executable.
set(RULES_URL https://rules2.clearurls.xyz/data.minify.json)
set(RULES_JSON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/app/pipeline/clearurls_rules.json)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    file(DOWNLOAD "${RULES_URL}" "${RULES_JSON_PATH}" SHOW_PROGRESS)
endif()

add_custom_command(TARGET ${PROJECT_SLUG} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RULES_JSON_PATH}"
        "$<TARGET_FILE_DIR:${PROJECT_SLUG}>/clearurls_rules.json")

add_custom_command(TARGET test1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RULES_JSON_PATH}"
        "$<TARGET_FILE_DIR:test1>/clearurls_rules.json")

target_link_libraries(${PROJECT_SLUG} PRIVATE ${COMMON_LIBS})
target_link_libraries(test1 PRIVATE ${COMMON_LIBS} GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

target_include_directories(${PROJECT_SLUG} PRIVATE ${COMMON_INCLUDES})
target_include_directories(test1 PRIVATE ${COMMON_INCLUDES})

if(WIN32)
    #target_sources(${PROJECT_SLUG} PRIVATE dpi-aware.manifest)
endif()

# "Install" target
set(CPACK_PACKAGE_NAME "BrowserTamer")
set(CPACK_PACKAGE_VENDOR "@aloneguid")
set(CPACK_PACKAGE_CONTACT "@aloneguid")
set(CPACK_PACKAGE_DISPLAY_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "BrowserTamer")
if(WIN32)
    install(TARGETS ${PROJECT_SLUG} RUNTIME DESTINATION ".")
    install(FILES "${RULES_JSON_PATH}" DESTINATION ".")

    # to use WIX generator (for producing .msi files) you can install wix toolset with .net sdk:
    # dotnet tool install --global wix
    #
    # then run the silly "eula accept" command:
    # wix eula accept wix7
    #
    # then install wix ui extension required by cpack:
    # wix extension add --global WixToolset.UI.wixext

    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64")
        set(CPACK_SYSTEM_NAME "win-arm64")
    else()
        set(CPACK_SYSTEM_NAME "win64")
    endif()

    set(CPACK_GENERATOR WIX)

    # "4" is required in order not to use the legacy WIX 3.x generator. It did not matter that I was using WIX 7.
    set(CPACK_WIX_VERSION 4)

    # WiX requires a stable Upgrade GUID — generate ONE and hardcode it (don't regenerate per build)
    set(CPACK_WIX_UPGRADE_GUID "C7630F02-56B6-41A6-8166-9D15AF6961E7")
    # needs to re-generate
    set(CPACK_WIX_PRODUCT_GUID "*")

    set(CPACK_WIX_INSTALL_SCOPE "perUser")
    set(CPACK_WIX_LICENSE_RTF "${CMAKE_SOURCE_DIR}/resources/LICENSE.rtf")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/icon.ico")
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/resources/wix/wix_banner.png")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/resources/wix/wix_dialog.png")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "Browser Tamer")
    set(CPACK_WIX_PROPERTY_APPHELPLINK "${PROJECT_HOMEPAGE_URL}")
    set(CPACK_WIX_PATCH_FILE "${CMAKE_SOURCE_DIR}/resources/wix/patch.xml")

    set_property(INSTALL "$<TARGET_FILE_NAME:bt>"
            PROPERTY CPACK_START_MENU_SHORTCUTS "${PROJECT_NAME}"
    )
elseif(UNIX AND NOT APPLE)
    install(TARGETS ${PROJECT_SLUG} RUNTIME DESTINATION "bin")
    install(FILES "${RULES_JSON_PATH}" DESTINATION "bin")
    set(CPACK_GENERATOR DEB)

    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libopengl0")

    install(FILES "${CMAKE_SOURCE_DIR}/resources/xdg/bt.desktop"
            DESTINATION share/applications)

    install(FILES "${CMAKE_SOURCE_DIR}/resources/icons/icon256x256.png"
            DESTINATION share/icons/hicolor/256x256/apps RENAME BrowserTamer.png)

    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
            "${CMAKE_SOURCE_DIR}/resources/xdg/postinst.sh;${CMAKE_SOURCE_DIR}/resources/xdg/postrm.sh")

elseif(APPLE)
    install(TARGETS ${PROJECT_SLUG} BUNDLE DESTINATION ".")
    set(CPACK_GENERATOR DragNDrop)
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${PROJECT_VERSION}-mac-${CMAKE_SYSTEM_PROCESSOR}")
    set(CPACK_DMG_VOLUME_NAME "${PROJECT_NAME}")
    set(CPACK_DMG_FORMAT UDZO)
    set(CPACK_DMG_DISABLE_APPLICATIONS_SYMLINK OFF)

endif()
include(CPack) # needs to be at the end of install configurations