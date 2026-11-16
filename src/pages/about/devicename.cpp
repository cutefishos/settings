// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/about/devicename.h"

#include <chrono>
#include <cstdio>
#include <unistd.h>

using namespace cutefish;
using namespace cutefish::base;

namespace settings {

DeviceName::DeviceName(dbus::Connection* bus)
    : m_service(bus, "org.freedesktop.hostname1", "/org/freedesktop/hostname1", "org.freedesktop.hostname1")
    , m_bluetooth(bluez::Manager::shared())
{
    if (!bus)
        return;
    // hostnamed only asks the polkit agent when the message itself allows it,
    // and the reply then waits for the password.
    m_service.setInteractiveAuthorizationAllowed(true);
    m_service.setTimeout(std::chrono::seconds(120));
    m_propertiesMatch = m_service.onPropertiesChanged(
            [this](const dbus::VariantMap& changedValues, const std::vector<std::string>&) {
                for (const auto& [name, value] : changedValues)
                    m_values[name] = value;
                changed.emit();
            });
    if (const dbus::Reply reply = m_service.getAll(); !reply.isError())
        m_values = reply.value<dbus::VariantMap>().value_or(dbus::VariantMap());
}

std::string DeviceName::value(const char* name) const
{
    const auto it = m_values.find(name);
    return it == m_values.end() ? std::string() : it->second.value<std::string>(std::string());
}

std::string DeviceName::chooseName(const std::string& pretty, const std::string& staticName,
                                   const std::string& hostname)
{
    if (!pretty.empty())
        return pretty;
    return staticName.empty() ? hostname : staticName;
}

std::string DeviceName::name() const
{
    std::string hostname = value("Hostname");
    if (hostname.empty()) {
        char buffer[256] = {};
        if (gethostname(buffer, sizeof(buffer) - 1) == 0)
            hostname = buffer;
    }
    return chooseName(value("PrettyHostname"), value("StaticHostname"), hostname);
}

void DeviceName::setName(const std::string& name)
{
    if (name.empty() || name == this->name())
        return;
    m_call = m_service.callAsync("SetPrettyHostname", [this, name](const dbus::Reply& reply) {
        if (reply.isError()) {
            fprintf(stderr, "Failed to set the device name: %s\n", reply.error().message.c_str());
            // The field goes back to the name the system keeps.
            changed.emit();
            return;
        }
        // BlueZ does not follow the pretty hostname once it runs.
        for (const auto& adapter : m_bluetooth->adapters())
            adapter->setName(name);
        if (value("PrettyHostname") != name) {
            // PropertiesChanged says so too, but may come later than the reply.
            m_values["PrettyHostname"] = name;
            changed.emit();
        }
    }, name, true);
}

} // namespace settings
