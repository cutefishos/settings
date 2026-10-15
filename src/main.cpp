// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstdio>
#include <string>

#include "base/dbus.h"
#include "ui/application.h"
#include "window/options.h"
#include "window/settingswindow.h"

using namespace cutefish;

static constexpr const char* kService = "com.cutefish.SettingsUI";
static constexpr const char* kPath = "/SettingsUI";

static void printHelp(const char* program)
{
    printf("Usage: %s [options]\n"
           "Cutefish Settings\n"
           "\n"
           "Options:\n"
           "  -h, --help                     Displays help on commandline options.\n"
           "  -m <module>                    Switch to module\n"
           "  --join-wifi <access point>     Join the Wi-Fi network of an access point\n"
           "  --mock-network                 Show sample network data\n"
           "  --mock-battery                 Show sample battery data\n",
           program);
}

int main(int argc, char** argv)
{
    std::string module;
    // The status bar's way to an enterprise network: the user name is asked for here.
    std::string accessPoint;
    settings::Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        const auto value = [&](const std::string& name, std::string& out) {
            if (argument == name) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "Missing value after '%s'.\n", argument.c_str());
                    exit(1);
                }
                out = argv[++i];
                return true;
            }
            if (argument.starts_with(name + "=")) {
                out = argument.substr(name.size() + 1);
                return true;
            }
            return false;
        };
        if (argument == "-h" || argument == "--help") {
            printHelp(argv[0]);
            return 0;
        } else if (value("-m", module) || value("--join-wifi", accessPoint)) {
        } else if (argument == "--mock-network") {
            options.mockNetwork = true;
        } else if (argument == "--mock-battery") {
            options.mockBattery = true;
        } else {
            fprintf(stderr, "Unknown option '%s'.\n", argument.c_str());
            return 1;
        }
    }
    settings::setOptions(options);

    ui::Application application(argc, argv);
    application.setApplicationId("cutefish-settings");

    // A running instance takes the request, and this process ends.
    base::dbus::Connection* bus = application.sessionBus();
    if (bus && !bus->requestName(kService)) {
        base::dbus::Proxy existing(bus, kService, kPath, kService);
        if (!accessPoint.empty())
            existing.call("joinWifi", accessPoint);
        else
            existing.call("switchToPage", module);
        return 0;
    }

    settings::SettingsWindow window;
    base::dbus::Object object(bus, kPath, kService);
    object.addMethod("switchToPage", [&window](const std::string& name) { window.switchToPage(name); });
    object.addMethod("joinWifi", [&window](const std::string& name) { window.joinWifi(name); });
    if (bus)
        object.publish();

    if (!accessPoint.empty())
        window.joinWifi(accessPoint);
    else if (!module.empty())
        window.switchToPage(module);
    return application.run();
}
