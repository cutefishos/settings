cmake_minimum_required(VERSION 3.22)
project(cutefish-shortcuts-model-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Core Gui DBus Test)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)
add_executable(shortcuts_model_test shortcuts_model_test.cpp
    ../../src/shortcuts/shortcutsmodel.cpp)
target_include_directories(shortcuts_model_test PRIVATE ../../src)
target_link_libraries(shortcuts_model_test PRIVATE Qt6::Core Qt6::Gui Qt6::DBus Qt6::Test)
enable_testing()
add_test(NAME shortcuts_model_test
    COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:shortcuts_model_test>)
