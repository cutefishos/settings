find_package(CutefishClients REQUIRED COMPONENTS packagekit)
target_link_libraries(settings-ui PUBLIC Cutefish::PackageKitClient)
