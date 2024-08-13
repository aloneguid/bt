cmake_minimum_required (VERSION 3.19)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(test)

find_package(fmt CONFIG REQUIRED)
find_package(Lua REQUIRED)
find_package(GTest CONFIG REQUIRED)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

enable_testing()

file(GLOB cpps CONFIGURE_DEPENDS
    "*.cpp"
    "../common/*.cpp"
    "../bt/app/match_rule.cpp"
    "../bt/app/security/*.cpp"
    "../bt/app/script_site.cpp")

add_executable(test ${cpps})

include(GoogleTest)
gtest_discover_tests(test)

target_link_libraries(test PRIVATE
    fmt::fmt-header-only
    GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main
    ${LUA_LIBRARIES})

target_include_directories(test PRIVATE
    "../common"
    ${LUA_INCLUDE_DIR})
