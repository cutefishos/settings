cmake_minimum_required(VERSION 3.22)
project(lifetime_test LANGUAGES CXX)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
find_package(Qt6 REQUIRED COMPONENTS Core Gui DBus Test)
find_package(PkgConfig REQUIRED)
pkg_check_modules(FontConfig REQUIRED IMPORTED_TARGET fontconfig)
add_executable(lifetime_test lifetime_test.cpp
    ../../src/fontsmodel.cpp
    ../../src/datetime/timezonemap.cpp
    ../../src/datetime/timedated_interface.cpp)
target_include_directories(lifetime_test PRIVATE ../../src)
# LeakSanitizer fails the run at exit when a backend leaves memory behind.
target_compile_options(lifetime_test PRIVATE -fsanitize=address -fno-omit-frame-pointer)
target_link_options(lifetime_test PRIVATE -fsanitize=address)
target_link_libraries(lifetime_test PRIVATE Qt6::Core Qt6::Gui Qt6::DBus Qt6::Test
    PkgConfig::FontConfig)
enable_testing()
add_test(NAME lifetime_test COMMAND lifetime_test)
