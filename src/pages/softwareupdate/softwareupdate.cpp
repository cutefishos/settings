// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/softwareupdate/softwareupdate.h"

#include <chrono>
#include <cstdint>
#include <tuple>

using namespace cutefish::base;

namespace settings {

static constexpr const char* kService = "org.cutefish.Services";
static constexpr const char* kPath = "/org/cutefish/Services/SoftwareUpdate";
static constexpr const char* kInterface = "org.cutefish.Services.SoftwareUpdate";

static dbus::Connection* s_packageKitBus = nullptr;

static SoftwareUpdate::State stateFromName(const std::string& name)
{
    if (name == "checking")
        return SoftwareUpdate::State::Checking;
    if (name == "up-to-date")
        return SoftwareUpdate::State::UpToDate;
    if (name == "available")
        return SoftwareUpdate::State::Available;
    if (name == "downloading")
        return SoftwareUpdate::State::Downloading;
    if (name == "ready")
        return SoftwareUpdate::State::Ready;
    if (name == "error")
        return SoftwareUpdate::State::Error;
    return SoftwareUpdate::State::Unknown;
}

SoftwareUpdate::SoftwareUpdate()
    : m_service(dbus::Connection::session(), kService, kPath, kInterface)
{
    dbus::Connection* bus = m_service.connection();
    if (!bus)
        return;
    // The restart waits for polkit, which may ask the user.
    m_service.setTimeout(std::chrono::minutes(2));
    m_changedMatch = m_service.onPropertiesChanged(
            [this](const dbus::VariantMap& changed, const std::vector<std::string>&) { apply(changed); });
    m_watch = bus->watchName(kService, [this](const std::string& owner) {
        if (!owner.empty())
            reload();
    });
}

void SoftwareUpdate::check()
{
    m_restartError.clear();
    m_service.send("check");
}

void SoftwareUpdate::download()
{
    m_restartError.clear();
    m_service.send("download");
}

void SoftwareUpdate::cancel()
{
    m_service.send("cancel");
}

void SoftwareUpdate::restartAndUpdate()
{
    m_restartError.clear();
    m_restart = m_service.callAsync("restartAndUpdate", [this](const dbus::Reply& reply) {
        if (!reply.isError())
            return;
        m_restartError = reply.error().message;
        changed.emit();
    });
}

dbus::Connection* SoftwareUpdate::packageKitBus()
{
    return s_packageKitBus ? s_packageKitBus : dbus::Connection::system();
}

void SoftwareUpdate::setPackageKitBus(dbus::Connection* bus)
{
    s_packageKitBus = bus;
}

void SoftwareUpdate::reload()
{
    m_reload = m_service.getAllAsync([this](const dbus::Reply& reply) {
        if (const auto properties = reply.value<dbus::VariantMap>())
            apply(*properties);
    });
}

void SoftwareUpdate::apply(const dbus::VariantMap& properties)
{
    if (const auto it = properties.find("state"); it != properties.end())
        m_state = stateFromName(it->second.value<std::string>());
    if (const auto it = properties.find("updates"); it != properties.end()) {
        m_updates.clear();
        for (auto& [name, version, summary] :
             it->second.value<std::vector<std::tuple<std::string, std::string, std::string>>>())
            m_updates.push_back({std::move(name), std::move(version), std::move(summary)});
    }
    if (const auto it = properties.find("progress"); it != properties.end())
        m_progress = it->second.value<int32_t>();
    if (const auto it = properties.find("error"); it != properties.end())
        m_error = it->second.value<std::string>();
    changed.emit();
}

} // namespace settings
