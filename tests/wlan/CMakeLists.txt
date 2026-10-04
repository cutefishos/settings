cmake_minimum_required(VERSION 3.22)
project(wlan_test LANGUAGES NONE)
enable_testing()

# Settings registers Metrics from its resources; the test gets it from a module of its own.
set(IMPORTS ${CMAKE_CURRENT_BINARY_DIR}/imports)
configure_file(../../src/qml/Metrics.qml ${IMPORTS}/Cutefish/Settings/Metrics.qml COPYONLY)
file(WRITE ${IMPORTS}/Cutefish/Settings/qmldir "module Cutefish.Settings\nsingleton Metrics 1.0 Metrics.qml\n")

find_program(QMLTESTRUNNER qmltestrunner HINTS /usr/lib/qt6/bin REQUIRED)
add_test(NAME wlan_join_dialog_test
         COMMAND ${QMLTESTRUNNER} -import ${IMPORTS} -input ${CMAKE_CURRENT_SOURCE_DIR}/tst_joindialog.qml)
set_tests_properties(wlan_join_dialog_test PROPERTIES ENVIRONMENT
    "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
