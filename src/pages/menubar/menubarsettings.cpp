// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/menubar/menubarsettings.h"

#include "base/paths.h"
#include "base/settings.h"

using namespace cutefish::base;

namespace settings {

MenuBarSettings::MenuBarSettings()
    : m_menuBar(dbus::Connection::session(), "org.cutefish.MenuBar", "/MenuBar", "org.cutefish.MenuBar")
{
    const Settings shell(paths::cutefishConfig("shell"));
    m_wifi = shell.boolValue("MenuBar/ShowWiFi", true);
    m_battery = shell.boolValue("MenuBar/ShowBattery", true);
    m_sound = shell.boolValue("MenuBar/ShowSound", true);
    m_bluetooth = shell.boolValue("MenuBar/ShowBluetooth", false);
}

bool MenuBarSettings::isControlShown(const std::string& control) const
{
    if (control == kWiFi)
        return m_wifi;
    if (control == kBattery)
        return m_battery;
    if (control == kSound)
        return m_sound;
    if (control == kBluetooth)
        return m_bluetooth;
    return false;
}

void MenuBarSettings::setControlShown(const std::string& control, bool shown)
{
    bool* value = control == kWiFi        ? &m_wifi
                  : control == kBattery   ? &m_battery
                  : control == kSound     ? &m_sound
                  : control == kBluetooth ? &m_bluetooth
                                          : nullptr;
    if (!value || *value == shown)
        return;
    m_menuBar.send("setControlShown", control, shown);
    *value = shown;
}

} // namespace settings
