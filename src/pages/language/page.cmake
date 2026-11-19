find_package(PkgConfig REQUIRED)
pkg_check_modules(SettingsLanguageIcu REQUIRED IMPORTED_TARGET icu-i18n icu-uc)
target_link_libraries(settings-core PUBLIC PkgConfig::SettingsLanguageIcu)
