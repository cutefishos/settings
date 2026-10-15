find_package(CutefishClients REQUIRED COMPONENTS timedate)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SettingsDateTimeIcu REQUIRED IMPORTED_TARGET icu-i18n icu-uc)
target_link_libraries(settings-ui PUBLIC Cutefish::TimeDateClient PkgConfig::SettingsDateTimeIcu)
