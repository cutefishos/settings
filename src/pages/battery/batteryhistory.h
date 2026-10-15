// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "upower/device.h"

namespace settings {

// A battery's charge or power draw over a span of time, from UPower's
// history, ready to plot: ordered by time, one sample per second at most,
// starting with the value in force at the span's start and running on to
// the present. With --mock-battery it holds sample days instead.
class BatteryHistory
{
public:
    enum class Type {
        Rate,
        Charge,
    };

    BatteryHistory();
    ~BatteryHistory();

    BatteryHistory(const BatteryHistory&) = delete;
    BatteryHistory& operator=(const BatteryHistory&) = delete;

    // The UPower device object path.
    void setDevice(const std::string& device);
    // Seconds back from now.
    void setDuration(uint32_t duration);
    uint32_t duration() const { return m_duration; }
    void setType(Type type);

    // Loads once set up, and again on every change of the above.
    void start();
    void refresh();

    const std::vector<cutefish::upower::HistorySample>& samples() const { return m_values; }
    int count() const { return int(m_values.size()); }
    uint32_t lastTime() const;
    int largestValue() const;

    cutefish::base::Signal<> dataChanged;

    // The samples to plot from UPower's answers: the span's samples, newest
    // first, and the whole history when it came, over `duration` seconds up
    // to `now`.
    static std::vector<cutefish::upower::HistorySample> plotted(
            const std::vector<cutefish::upower::HistorySample>& window,
            const std::optional<std::vector<cutefish::upower::HistorySample>>& all, uint32_t now,
            uint32_t duration);

private:
    void load();
    void loadMock();
    void finish();

    std::string m_device;
    uint32_t m_duration = 120;
    Type m_type = Type::Charge;
    bool m_started = false;
    std::unique_ptr<cutefish::upower::Device> m_upowerDevice;
    std::vector<cutefish::upower::HistorySample> m_values;

    cutefish::base::dbus::PendingCall m_windowCall;
    cutefish::base::dbus::PendingCall m_allCall;
    int m_pending = 0;
    std::optional<std::vector<cutefish::upower::HistorySample>> m_window;
    std::optional<std::vector<cutefish::upower::HistorySample>> m_all;
};

} // namespace settings
