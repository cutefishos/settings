// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/battery/batteryhistory.h"

#include <algorithm>
#include <cstdio>
#include <ctime>
#include <limits>

#include "window/options.h"

using cutefish::upower::Device;
using cutefish::upower::HistorySample;

namespace settings {

BatteryHistory::BatteryHistory() = default;

BatteryHistory::~BatteryHistory() = default;

void BatteryHistory::setDevice(const std::string& device)
{
    if (device == m_device)
        return;

    m_device = device;
    m_upowerDevice.reset();
    load();
}

void BatteryHistory::setDuration(uint32_t duration)
{
    if (duration == m_duration)
        return;

    m_duration = duration;
    load();
}

void BatteryHistory::setType(Type type)
{
    if (type == m_type)
        return;

    m_type = type;
    load();
}

void BatteryHistory::start()
{
    m_started = true;
    load();
}

void BatteryHistory::refresh()
{
    load();
}

uint32_t BatteryHistory::lastTime() const
{
    return m_values.empty() ? 0 : m_values.back().time;
}

int BatteryHistory::largestValue() const
{
    int max = 0;
    for (const HistorySample& sample : m_values) {
        if (sample.value > max)
            max = int(sample.value);
    }
    return max;
}

void BatteryHistory::load()
{
    if (!m_started || m_device.empty())
        return;

    if (options().mockBattery) {
        loadMock();
        return;
    }

    if (!m_upowerDevice)
        m_upowerDevice = std::make_unique<Device>(m_device);

    const Device::HistoryType type = m_type == Type::Rate ? Device::HistoryType::Rate : Device::HistoryType::Charge;
    m_window.reset();
    m_all.reset();
    m_pending = 2;

    m_windowCall = m_upowerDevice->history(type, m_duration, 100,
                                           [this](const std::optional<std::vector<HistorySample>>& samples) {
                                               m_window = samples;
                                               if (--m_pending == 0)
                                                   finish();
                                           });

    // UPower returns only the samples inside the timespan and records one only
    // when the value changes, so the value at the window's start is the newest
    // sample before it, found in the whole history.
    m_allCall = m_upowerDevice->history(type, std::numeric_limits<uint32_t>::max(),
                                        std::numeric_limits<uint32_t>::max(),
                                        [this](const std::optional<std::vector<HistorySample>>& samples) {
                                            m_all = samples;
                                            if (--m_pending == 0)
                                                finish();
                                        });
}

void BatteryHistory::finish()
{
    m_values.clear();

    if (!m_window) {
        fprintf(stderr, "Failed to get device history from UPower\n");
        return;
    }

    m_values = plotted(*m_window, m_all, uint32_t(std::time(nullptr)), m_duration);
    dataChanged.emit();
}

std::vector<HistorySample> BatteryHistory::plotted(const std::vector<HistorySample>& window,
                                                   const std::optional<std::vector<HistorySample>>& all,
                                                   uint32_t now, uint32_t duration)
{
    std::vector<HistorySample> values;
    const uint32_t start = now > duration ? now - duration : 0;

    // The sample in force at the start. Restart markers are skipped here as
    // they are inside the window, or the start goes blank once one follows it.
    if (all) {
        HistorySample before;

        for (auto it = all->crbegin(); it != all->crend(); ++it) {
            if (it->time > start)
                break;

            if (it->value <= 0 || it->time < before.time)
                continue;

            before = *it;
        }

        if (before.value > 0) {
            before.time = start;
            values.push_back(before);
        }
    }

    // UPower returns the newest sample first, in the order it recorded them.
    // A sample older than the one recorded before it was stamped while the
    // clock was wrong (before NTP set it after boot), so it is dropped.
    for (auto it = window.crbegin(); it != window.crend(); ++it) {
        // A zero value marks a UPower restart, not a reading.
        if (it->value <= 0 || it->time > now)
            continue;

        if (!values.empty() && it->time < values.back().time)
            continue;

        // The curve needs distinct times; of two readings in a second, keep the later.
        if (!values.empty() && it->time == values.back().time) {
            values.back() = *it;
            continue;
        }

        values.push_back(*it);
    }

    // UPower records a sample only when the value changes, so the last one
    // still holds and the plot runs on to the present.
    if (!values.empty() && values.back().time < now) {
        HistorySample current = values.back();
        current.time = now;
        values.push_back(current);
    }

    return values;
}

void BatteryHistory::loadMock()
{
    m_values.clear();
    const uint32_t now = uint32_t(std::time(nullptr));
    // Each day drains on battery, charges, is held at 90% by the charge
    // limit and is unplugged again, ending where the next day starts.
    static const std::vector<double> chargeValues{72, 66, 59, 52, 45, 38, 32, 44, 57,
                                                  69, 80, 88, 90, 90, 86, 81, 77, 72};
    static const std::vector<double> rateValues{7.2, 6.8, 7.5, 6.9, 7.1, 6.4, 6.0, 11.8, 12.4,
                                                11.1, 9.6, 7.2, 4.1, 3.9, 6.8, 7.4, 6.9, 6.6};
    static const std::vector<uint32_t> states{2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1, 5, 2, 2, 2, 2};
    // Days drain a little deeper or shallower than one another.
    static const std::vector<double> depths{0, -6, 4, -3, 6, -4, 2};
    const std::vector<double>& values = m_type == Type::Rate ? rateValues : chargeValues;
    const uint32_t days = std::max(1u, m_duration / 86400);
    const uint32_t interval = m_duration / days / uint32_t(values.size());

    for (uint32_t day = 0; day < days; ++day) {
        for (size_t i = 0; i < values.size(); ++i) {
            HistorySample sample;
            sample.time = now - m_duration + (day * uint32_t(values.size()) + uint32_t(i)) * interval;
            sample.value = values[i];
            sample.state = states[i];

            if (m_type == Type::Charge && i >= 3 && i <= 8)
                sample.value += depths[(days - 1 - day) % depths.size()];

            m_values.push_back(sample);
        }
    }

    HistorySample current = m_values.back();
    current.time = now;
    m_values.push_back(current);

    dataChanged.emit();
}

} // namespace settings
