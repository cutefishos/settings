// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"

namespace settings {

// When cutefish-services turns the display off and whether the computer
// may sleep then. The values are read from the service's settings file;
// changes go to the service, which owns them.
class PowerSettings
{
public:
    // Seconds; -1 is never.
    static constexpr int kDefaultBatteryScreenOff = 300;
    static constexpr int kDefaultACScreenOff = 1200;

    PowerSettings();

    int batteryScreenOff() const { return m_batteryScreenOff; }
    void setBatteryScreenOff(int timeout);
    int acScreenOff() const { return m_acScreenOff; }
    void setACScreenOff(int timeout);
    bool acPreventSleep() const { return m_acPreventSleep; }
    void setACPreventSleep(bool prevent);

private:
    cutefish::base::dbus::Proxy m_service;
    int m_batteryScreenOff;
    int m_acScreenOff;
    bool m_acPreventSleep;
};

} // namespace settings
