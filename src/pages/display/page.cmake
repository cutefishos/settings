find_package(CutefishClients REQUIRED COMPONENTS display)
target_link_libraries(settings-core PUBLIC Cutefish::DisplayClient)
