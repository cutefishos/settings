find_package(CutefishClients REQUIRED COMPONENTS accounts)
find_library(SETTINGS_CRYPT_LIBRARY crypt REQUIRED)
target_link_libraries(settings-ui PUBLIC Cutefish::AccountsClient ${SETTINGS_CRYPT_LIBRARY})
