find_package(GTest REQUIRED)
include(GoogleTest)

# A layer's tests: each page adds its own <page>_test.cpp or
# <page>_<topic>_test.cpp to a layer's directory, built with the page.
function(settings_page_tests variable directory)
    file(GLOB tests CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${directory}/*_test.cpp)
    set(wanted_tests)
    foreach(test ${tests})
        get_filename_component(name ${test} NAME_WE)
        set(wanted FALSE)
        if(name STREQUAL "foundation_test")
            set(wanted TRUE)
        endif()
        foreach(page ${SETTINGS_BUILT_PAGES})
            if(name STREQUAL "${page}_test" OR name MATCHES "^${page}_.*_test$")
                set(wanted TRUE)
            endif()
        endforeach()
        if(wanted)
            list(APPEND wanted_tests ${test})
        endif()
    endforeach()
    set(${variable} ${wanted_tests} PARENT_SCOPE)
endfunction()

# Each test on a session bus of its own, where the pages' fakes own their
# services' names and the session's real services are out of reach.
if(CMAKE_VERSION VERSION_LESS 3.29)
    message(FATAL_ERROR "The settings tests need CMake 3.29 for TEST_LAUNCHER")
endif()
find_program(DBUS_RUN_SESSION_EXECUTABLE dbus-run-session REQUIRED)
set(SETTINGS_TEST_LAUNCHER
    "${DBUS_RUN_SESSION_EXECUTABLE};--config-file=${CMAKE_CURRENT_SOURCE_DIR}/support/dbus-session.conf;--")

# The models alone: no toolkit ui, so no window can come in.
settings_page_tests(logic_tests logic)
add_executable(settings-logic-tests logic/main.cpp ${logic_tests})
target_include_directories(settings-logic-tests PRIVATE support)
target_link_libraries(settings-logic-tests PRIVATE settings-core GTest::gtest)
set_target_properties(settings-logic-tests PROPERTIES TEST_LAUNCHER "${SETTINGS_TEST_LAUNCHER}")
gtest_discover_tests(settings-logic-tests PROPERTIES LABELS logic)

# What the user does and then sees, through the toolkit's test driver.
settings_page_tests(scenario_tests scenarios)
add_executable(settings-scenario-tests scenarios/main.cpp ${scenario_tests})
target_compile_definitions(settings-scenario-tests PRIVATE SETTINGS_SOURCE_DIR="${PROJECT_SOURCE_DIR}/src")
target_include_directories(settings-scenario-tests PRIVATE support)
target_link_libraries(settings-scenario-tests PRIVATE settings-ui GTest::gtest)
set_target_properties(settings-scenario-tests PROPERTIES TEST_LAUNCHER "${SETTINGS_TEST_LAUNCHER}")
gtest_discover_tests(settings-scenario-tests PROPERTIES LABELS scenario)

# Geometry, spacing, centring and pixels.
settings_page_tests(layout_tests layout)
add_executable(settings-layout-tests layout/main.cpp ${layout_tests})
target_compile_definitions(settings-layout-tests PRIVATE SETTINGS_SOURCE_DIR="${PROJECT_SOURCE_DIR}/src")
target_include_directories(settings-layout-tests PRIVATE support)
target_link_libraries(settings-layout-tests PRIVATE settings-ui GTest::gtest)
set_target_properties(settings-layout-tests PROPERTIES TEST_LAUNCHER "${SETTINGS_TEST_LAUNCHER}")
gtest_discover_tests(settings-layout-tests PROPERTIES LABELS layout)

# Renders every page headless into PNGs, light and dark, at 100% and 150%.
add_executable(settings-capture layout/capture.cpp)
target_compile_definitions(settings-capture PRIVATE SETTINGS_SOURCE_DIR="${PROJECT_SOURCE_DIR}/src")
target_link_libraries(settings-capture PRIVATE settings-ui)
