cmake_minimum_required(VERSION 3.16)

project(cutefish-settings
    VERSION 1.0.0
    LANGUAGES CXX
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(QT Core Gui Quick QuickControls2 DBus Concurrent LinguistTools)
find_package(Qt6 REQUIRED ${QT})
find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(Xcursor REQUIRED IMPORTED_TARGET xcursor)
find_package(Libcrypt REQUIRED)


find_package(CutefishFramework REQUIRED COMPONENTS Application Appearance IO Power)

pkg_search_module(FontConfig REQUIRED fontconfig IMPORTED_TARGET)
pkg_search_module(ICU REQUIRED icu-i18n)

include_directories(${ICU_INCLUDE_DIRS})

set(SRCS
    src/application.cpp
    src/main.cpp
    src/docksettings.cpp
    src/battery.cpp
    src/brightness.cpp
    src/fontsmodel.cpp
    src/language.cpp
    src/about.cpp
    src/notifications.cpp
    src/password.cpp
    src/batteryhistorymodel.cpp
    src/icontheme/iconthememodel.cpp
    src/fonts/fonts.cpp
    src/powermanager.cpp
    src/cursor/cursorthememodel.cpp
    src/cursor/cursortheme.cpp
    src/cursor/mouse.cpp
    src/datetime/time.cpp
    src/datetime/timezonemap.cpp
    src/datetime/timedated_interface.cpp
    src/touchpad.cpp
    src/touchscreen.cpp
    src/shortcuts/shortcutsmodel.cpp
    src/networkproxy.cpp
    src/defaultapplications.cpp
    src/inputmethod.cpp
    src/keyboard.cpp
    src/mock/networkmockmodel.cpp
)

set(RESOURCES
    src/resources.qrc
)

qt6_add_dbus_adaptor(DBUS_SOURCES
                     src/com.cutefish.SettingsUI.xml
                     src/application.h Application)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(${PROJECT_NAME} ${DBUS_SOURCES} ${SRCS} ${RESOURCES})
target_link_libraries(${PROJECT_NAME}
        PkgConfig::Xcursor
        Cutefish::Application
        Cutefish::Appearance
        Cutefish::IO
        Cutefish::Power
        Qt6::Core
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::DBus
        Qt6::Concurrent

        PkgConfig::FontConfig
        ${FREETYPE_LIBRARIES}
        ${ICU_LDFLAGS}
        Libcrypt::Libcrypt

)

file(GLOB TS_FILES translations/*.ts)
qt6_add_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${TS_FILES})
add_dependencies(${PROJECT_NAME} translations)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
    cutefish-settings.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)

install(FILES ${QM_FILES} DESTINATION /usr/share/${PROJECT_NAME}/translations)
