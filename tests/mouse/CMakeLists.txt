cmake_minimum_required(VERSION 3.22)
project(cutefish-mouse-integration-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Core DBus Test)
find_program(DBUS_RUN_SESSION dbus-run-session REQUIRED)
set(COMPOSITOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../cutefish-compositor" CACHE PATH
    "Compositor source tree")
add_executable(mouse_integration_test mouse_integration_test.cpp
    ../../src/cursor/mouse.cpp
    ${COMPOSITOR_DIR}/src/input/mouse_settings.cpp)
target_include_directories(mouse_integration_test PRIVATE ../../src ${COMPOSITOR_DIR}/src)
target_link_libraries(mouse_integration_test PRIVATE Qt6::Core Qt6::DBus Qt6::Test)
enable_testing()
add_test(NAME mouse_integration_test
    COMMAND ${DBUS_RUN_SESSION} -- $<TARGET_FILE:mouse_integration_test>)
