// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>

#include "base/dbus.h"

namespace settings {

// Which controls the menu bar shows: read from the shell's file, changed
// through the menu bar, which owns that file.
class MenuBarSettings
{
public:
    // The names org.cutefish.MenuBar takes.
    static constexpr const char* kWiFi = "wifi";
    static constexpr const char* kBattery = "battery";
    static constexpr const char* kSound = "sound";
    static constexpr const char* kBluetooth = "bluetooth";

    MenuBarSettings();

    bool isControlShown(const std::string& control) const;
    void setControlShown(const std::string& control, bool shown);

private:
    cutefish::base::dbus::Proxy m_menuBar;
    bool m_wifi = true;
    bool m_battery = true;
    bool m_sound = true;
    bool m_bluetooth = false;
};

} // namespace settings
