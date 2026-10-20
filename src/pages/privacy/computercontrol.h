// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace settings {

// The compositor's org.cutefish.ComputerControl1: the apps whose agents may
// control the computer without asking.
class ComputerControl
{
public:
    struct App
    {
        std::string path;
        // The agent's name for itself, and its application's desktop entry.
        std::string name;
        std::string appId;
    };

    ComputerControl();

    bool isAvailable() const { return m_available; }
    const std::vector<App>& allowedApps() const { return m_apps; }
    void forget(const std::string& path);

    cutefish::base::Signal<> changed;

private:
    void reload();

    cutefish::base::dbus::Proxy m_control;
    cutefish::base::dbus::NameWatch m_watch;
    cutefish::base::dbus::Match m_changedMatch;
    cutefish::base::dbus::PendingCall m_reload;
    cutefish::base::dbus::PendingCall m_forget;
    std::vector<App> m_apps;
    bool m_available = false;
};

} // namespace settings
