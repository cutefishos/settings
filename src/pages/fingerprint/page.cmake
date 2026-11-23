find_package(CutefishClients REQUIRED COMPONENTS fprint)
target_link_libraries(settings-core PUBLIC Cutefish::FprintClient)
