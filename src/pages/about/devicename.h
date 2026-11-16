// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>
#include <string>

#include "base/dbus.h"
#include "base/signal.h"
#include "cutefish/bluez/manager.h"

namespace settings {

// The name the computer shows to people, as org.freedesktop.hostname1 keeps
// it, which Bluetooth adapters take as theirs too.
class DeviceName
{
public:
    explicit DeviceName(cutefish::base::dbus::Connection* bus = cutefish::base::dbus::Connection::system());

    DeviceName(const DeviceName&) = delete;
    DeviceName& operator=(const DeviceName&) = delete;

    std::string name() const;
    // The service may ask polkit first; the name changes once it says so.
    void setName(const std::string& name);

    // The pretty name, else the static hostname, else the kernel's.
    static std::string chooseName(const std::string& pretty, const std::string& staticName,
                                  const std::string& hostname);

    cutefish::base::Signal<> changed;

private:
    std::string value(const char* name) const;

    cutefish::base::dbus::Proxy m_service;
    cutefish::base::dbus::Match m_propertiesMatch;
    cutefish::base::dbus::PendingCall m_call;
    cutefish::base::dbus::VariantMap m_values;
    // Kept so the adapters are known by the time the name changes.
    std::shared_ptr<cutefish::bluez::Manager> m_bluetooth;
};

} // namespace settings
