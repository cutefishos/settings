find_package(CutefishClients REQUIRED COMPONENTS upower powerprofiles)
target_link_libraries(settings-core PUBLIC Cutefish::UPowerClient Cutefish::PowerProfilesClient)
