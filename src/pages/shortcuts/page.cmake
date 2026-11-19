find_package(PkgConfig REQUIRED)
pkg_check_modules(SettingsShortcuts REQUIRED IMPORTED_TARGET xkbcommon)
target_link_libraries(settings-core PUBLIC PkgConfig::SettingsShortcuts)
