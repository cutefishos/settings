find_package(CutefishClients REQUIRED COMPONENTS upower)
target_link_libraries(settings-core PUBLIC Cutefish::UPowerClient)
