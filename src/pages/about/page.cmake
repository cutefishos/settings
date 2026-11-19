find_package(CutefishClients REQUIRED COMPONENTS bluez)
target_link_libraries(settings-core PUBLIC Cutefish::BluezClient)
