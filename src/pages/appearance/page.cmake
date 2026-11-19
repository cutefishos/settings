find_package(CutefishClients REQUIRED COMPONENTS appearance)
target_link_libraries(settings-core PUBLIC Cutefish::AppearanceClient)
