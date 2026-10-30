// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/battery/battery.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "base/file.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/settings.h"
#include "cutefish/upower/battery_status.h"
#include "cutefish/upower/device.h"
#include "pages/battery/timeformat.h"
#include "window/options.h"

using namespace cutefish;
using namespace cutefish::base;
using cutefish::i18n::format;
using cutefish::i18n::tr;

namespace settings {

static int64_t now()
{
    return int64_t(std::time(nullptr));
}

Battery::Battery()
    : m_mock(options().mockBattery)
{
    if (m_mock) {
        m_batteries = {"mock-battery"};
        if (options().mockExtraBattery)
            m_batteries.push_back("mock-battery-extra");
        m_chargeLimitSupported = true;
        m_chargeLimitStart = 75;
        m_chargeLimitEnd = 80;
        return;
    }

    m_status = upower::BatteryStatus::shared();
    m_subscriptions.push_back(m_status->readyChanged.connect([this] { readyChanged.emit(); }));
    m_subscriptions.push_back(m_status->availableChanged.connect([this] { availableChanged.emit(); }));
    m_subscriptions.push_back(m_status->lastChargedChanged.connect([this] { lastChargedChanged.emit(); }));
    m_subscriptions.push_back(m_status->batteriesChanged.connect([this] { watchBatteries(); }));

    m_recheck.setSingleShot(true);
    m_recheck.onTimeout = [this] { readKernelChargeLimits(); };

    watchBatteries();
}

Battery::~Battery() = default;

void Battery::watchBatteries()
{
    if (m_status->batteries() == m_batteries)
        return;

    // The picked battery stays picked wherever it lands in the new list.
    const std::string picked = udi();

    m_deviceSubscriptions.clear();
    m_devices.clear();
    m_batteries = m_status->batteries();
    m_kernelChargeLimits.assign(m_batteries.size(), -1);

    for (size_t i = 0; i < m_batteries.size(); ++i) {
        auto device = std::make_unique<upower::Device>(m_batteries[i]);
        m_deviceSubscriptions.push_back(device->propertiesChanged.connect([this, i](const dbus::VariantMap&) {
            if (int(i) == m_current)
                capacityChanged.emit();
            updateChargeLimit();
            // UPower updates the battery every little while; a cheap moment to look again.
            readKernelChargeLimits();
        }));
        m_devices.push_back(std::move(device));
    }

    const auto found = std::find(m_batteries.begin(), m_batteries.end(), picked);
    m_current = found == m_batteries.end() ? 0 : int(found - m_batteries.begin());
    batteriesChanged.emit();
    if (udi() != picked)
        udiChanged.emit();
    capacityChanged.emit();
    updateChargeLimit();
}

void Battery::setCurrentBattery(int index)
{
    if (index == m_current || index < 0 || index >= int(m_batteries.size()))
        return;

    m_current = index;
    udiChanged.emit();
    capacityChanged.emit();
}

std::string Battery::udi() const
{
    return m_current < int(m_batteries.size()) ? m_batteries[m_current] : std::string();
}

bool Battery::isReady() const
{
    return m_mock || m_status->isReady();
}

bool Battery::isAvailable() const
{
    return m_mock || m_status->isAvailable();
}

int Battery::lastChargedPercent() const
{
    return m_mock ? 100 : m_status->lastChargedPercent();
}

int Battery::capacity() const
{
    if (m_mock)
        return m_current == 0 ? 94 : 81;
    if (m_current >= int(m_devices.size()))
        return 0;

    const dbus::VariantMap& properties = m_devices[m_current]->properties();
    const auto it = properties.find("Capacity");
    return it == properties.end() ? 0 : int(it->second.value<double>(0));
}

std::string Battery::lastChargedTime() const
{
    const int64_t secs = m_mock ? now() - 2 * 3600 : m_status->lastChargedSecs();

    if (secs <= 0)
        return {};

    // A date and time, as the file manager shows them; the service's own
    // "13 h 15 min ago" is worked out once and goes stale on an open page.
    const int64_t days = timeformat::daysBetween(secs, now());

    if (days == 0)
        return format(tr("Today at {time}"), {{"time", timeformat::shortTime(secs)}});
    if (days == 1)
        return format(tr("Yesterday at {time}"), {{"time", timeformat::shortTime(secs)}});

    return timeformat::shortDateTime(secs);
}

bool Battery::showPercent() const
{
    if (m_mock)
        return true;

    Settings settings(paths::cutefishConfig("shell"));
    return settings.boolValue("Statusbar/BatteryPercentage", false);
}

void Battery::setPercentEnabled(bool enabled)
{
    if (m_mock)
        return;

    dbus::Proxy(dbus::Connection::session(), "org.cutefish.Statusbar", "/Statusbar", "org.cutefish.Statusbar")
            .send("setBatteryPercentage", enabled);
}

bool Battery::chargeLimitHeld() const
{
    if (!m_chargeLimitEnabled)
        return true;

    for (size_t i = 0; i < m_devices.size(); ++i) {
        const upower::Device& device = *m_devices[i];
        if (!device.chargeThresholdSupported())
            continue;
        // A battery that refused the limit charges fully as well.
        if (!device.chargeThresholdEnabled())
            return false;
        if (m_kernelChargeLimits[i] >= 0 && m_kernelChargeLimits[i] != device.chargeEndThreshold())
            return false;
    }
    return true;
}

void Battery::setChargeLimitEnabled(bool enabled)
{
    if (m_mock) {
        m_chargeLimitEnabled = enabled;
        chargeLimitChanged.emit();
        return;
    }

    for (const auto& device : m_devices) {
        if (!device->chargeThresholdSupported())
            continue;

        device->enableChargeThreshold(enabled, [this](const dbus::Error& error) {
            if (error.isSet()) {
                fprintf(stderr, "Failed to change the charge limit: %s\n", error.message.c_str());
                // Puts a switch showing chargeLimitEnabled() back.
                chargeLimitChanged.emit();
            }
        });
    }
}

void Battery::readKernelChargeLimits()
{
    bool changed = false;

    for (size_t i = 0; i < m_devices.size(); ++i) {
        const std::string name = m_devices[i]->nativePath();
        if (name.empty())
            continue;

        int limit = -1;
        if (const auto contents = file::read("/sys/class/power_supply/" + name + "/charge_control_end_threshold"))
            limit = std::atoi(contents->c_str());

        if (limit != m_kernelChargeLimits[i]) {
            m_kernelChargeLimits[i] = limit;
            changed = true;
        }
    }

    if (changed)
        chargeLimitChanged.emit();
}

// UPower before 1.90 has none of these properties, which leaves the limit
// unsupported. The thresholds shown are the first battery's with a limit.
void Battery::updateChargeLimit()
{
    bool supported = false;
    bool enabled = false;
    int start = 0;
    int end = 0;

    for (const auto& device : m_devices) {
        if (!device->chargeThresholdSupported())
            continue;

        if (!supported) {
            start = device->chargeStartThreshold();
            end = device->chargeEndThreshold();
        }
        supported = true;
        enabled = enabled || device->chargeThresholdEnabled();
    }

    if (supported == m_chargeLimitSupported && enabled == m_chargeLimitEnabled && start == m_chargeLimitStart
        && end == m_chargeLimitEnd)
        return;

    const bool turnedOn = enabled && !m_chargeLimitEnabled;

    m_chargeLimitSupported = supported;
    m_chargeLimitEnabled = enabled;
    m_chargeLimitStart = start;
    m_chargeLimitEnd = end;
    chargeLimitChanged.emit();

    // A controller that rejects the thresholds resets them within seconds,
    // without UPower or the kernel announcing it.
    if (turnedOn)
        m_recheck.start(std::chrono::seconds(5));
}

} // namespace settings
