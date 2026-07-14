cmake_minimum_required (VERSION 3.20)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)
find_path(P_RANAV_CSV2_INCLUDE_DIRS "csv2/mio.hpp")
find_package(tinyxml2 CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

include(GNUInstallDirs)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
enable_testing()

file(GLOB core_src
    "globals.cpp"
    "cmdline.cpp"
    "state.cpp"
    "app/*.cpp"
    "app/ui/*.cpp"
    "app/pipeline/*.cpp"
    "../grey/grey/common/fss.cpp"
    "../grey/grey/common/http.cpp"
    "../grey/grey/common/url.cpp"
    "../grey/grey/common/datetime.cpp"
    "../grey/grey/common/desktop_shell.cpp"
    "../grey/grey/common/process.cpp"
    "../grey/grey/common/hashing.cpp")

if(WIN32)
file(GLOB core_src
    ${core_src}
    "../grey/grey/common/win32/window.cpp"
    "../grey/grey/common/win32/shell.cpp"
    "../grey/grey/common/win32/os.cpp"
    "../grey/grey/common/win32/reg.cpp"
    "../grey/grey/common/win32/user.cpp")

    add_executable (${APP_NAME} WIN32 ${core_src} "bt.cpp" bt.rc)
else()
    add_executable (${APP_NAME} ${core_src} "bt.cpp")
endif()

file(GLOB test_src ${core_src} "tests/*.cpp")
add_executable(test ${test_src})
include(GoogleTest)
gtest_discover_tests(test)

set(COMMON_LIBS
        libgrey
        nlohmann_json::nlohmann_json
        tinyxml2::tinyxml2
        unofficial::sqlite3::sqlite3
        ${LUA_LIBRARIES})

set(COMMON_INCLUDES
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "../grey/grey/common"
        "../grey/grey"
        ${P_RANAV_CSV2_INCLUDE_DIRS}
        ${LUA_INCLUDE_DIR})

target_link_libraries(${APP_NAME} PRIVATE ${COMMON_LIBS})
target_link_libraries(test PRIVATE ${COMMON_LIBS} GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

target_include_directories(${APP_NAME} PRIVATE ${COMMON_INCLUDES})
target_include_directories(test PRIVATE ${COMMON_INCLUDES})

if(WIN32)
    target_sources(${APP_NAME} PRIVATE dpi-aware.manifest)

    # only BT needs windows-specific main function, in order to capture UNICODE command line parameters and not to spawn terminal
    target_link_options(${APP_NAME} PRIVATE
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

# "Install" target
set(CPACK_PACKAGE_NAME "browsertamer")
set(CPACK_PACKAGE_VENDOR "@aloneguid")
set(CPACK_PACKAGE_CONTACT "@aloneguid")
set(CPACK_PACKAGE_DISPLAY_NAME ${APP_LONG_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "BrowserTamer")
if(WIN32)
    install(TARGETS ${APP_NAME} RUNTIME DESTINATION ".")

    # to use WIX generator (for producing .msi files) you can install wix toolset with .net sdk:
    # dotnet tool install --global wix
    #
    # then run the silly "eula accept" command:
    # wix eula accept wix7
    #
    # then install wix ui extension required by cpack:
    # wix extension add --global WixToolset.UI.wixext

    set(CPACK_GENERATOR WIX)

    # "4" is required in order not to use the legacy WIX 3.x generator. It did not matter that I was using WIX 7.
    set(CPACK_WIX_VERSION 4)

    # WiX requires a stable Upgrade GUID — generate ONE and hardcode it (don't regenerate per build)
    set(CPACK_WIX_UPGRADE_GUID "C7630F02-56B6-41A6-8166-9D15AF6961E7")
    # needs to re-generate
    set(CPACK_WIX_PRODUCT_GUID "*")

    set(CPACK_WIX_INSTALL_SCOPE "perUser")
    set(CPACK_WIX_LICENSE_RTF "${CMAKE_SOURCE_DIR}/resources/LICENSE.rtf")
    set(CPACK_WIX_PRODUCT_ICON "${CMAKE_SOURCE_DIR}/bt/icon.ico")
    set(CPACK_WIX_UI_BANNER "${CMAKE_SOURCE_DIR}/resources/wix/wix_banner.png")
    set(CPACK_WIX_UI_DIALOG "${CMAKE_SOURCE_DIR}/resources/wix/wix_dialog.png")
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "Browser Tamer")
    set(CPACK_WIX_PROPERTY_APPHELPLINK "${APP_URL}")
    set(CPACK_WIX_PATCH_FILE "${CMAKE_SOURCE_DIR}/resources/wix/patch.xml")

    set_property(INSTALL "$<TARGET_FILE_NAME:bt>"
            PROPERTY CPACK_START_MENU_SHORTCUTS "${APP_LONG_NAME}"
    )
elseif(UNIX AND NOT APPLE)
    install(TARGETS ${APP_NAME} RUNTIME DESTINATION "bin")
    set(CPACK_GENERATOR DEB)

    set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

    install(FILES "${CMAKE_SOURCE_DIR}/resources/xdg/bt.desktop"
            DESTINATION share/applications)

    install(FILES "${CMAKE_SOURCE_DIR}/resources/icons/icon256x256.png"
            DESTINATION share/icons/hicolor/256x256/apps RENAME BrowserTamer.png)

    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
            "${CMAKE_SOURCE_DIR}/resources/xdg/postinst;${CMAKE_SOURCE_DIR}/resources/xdg/postrm")

endif()
include(CPack) # needs to be at the end of install configurations