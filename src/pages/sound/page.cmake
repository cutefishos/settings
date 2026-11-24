find_package(CutefishClients REQUIRED COMPONENTS audio bluez)
target_link_libraries(settings-core PUBLIC Cutefish::AudioClient Cutefish::BluezClient)
