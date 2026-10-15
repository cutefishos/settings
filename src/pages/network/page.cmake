find_package(CutefishClients REQUIRED COMPONENTS networkmanager)
target_link_libraries(settings-ui PUBLIC Cutefish::NetworkManagerClient)
