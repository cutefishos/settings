find_package(CutefishClients REQUIRED COMPONENTS audio)
target_link_libraries(settings-core PUBLIC Cutefish::AudioClient)
