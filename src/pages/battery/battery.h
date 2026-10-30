// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"
#include "base/timer.h"

namespace cutefish::upower {
class BatteryStatus;
class Device;
}

namespace settings {

// The battery page's view of cutefish-services' batteries, plus each
// battery's capacity and UPower's charge limit. With --mock-battery it shows
// sample values instead.
class Battery
{
public:
    Battery();
    ~Battery();

    Battery(const Battery&) = delete;
    Battery& operator=(const Battery&) = delete;

    bool isReady() const;
    bool isAvailable() const;

    // The UPower device object paths of the batteries; the page shows the
    // history and health of the current one.
    const std::vector<std::string>& batteries() const { return m_batteries; }
    int currentBattery() const { return m_current; }
    void setCurrentBattery(int index);
    // The current battery's UPower device object path.
    std::string udi() const;

    int lastChargedPercent() const;
    // The current battery's.
    int capacity() const;
    // "Today at 2:21 pm", or the date and time; empty when never charged.
    std::string lastChargedTime() const;

    // Whether the status bar shows the percentage.
    bool showPercent() const;
    void setPercentEnabled(bool enabled);

    // UPower's charge limit: only on or off, between thresholds the system
    // sets. It is turned on and off for every battery that has one.
    bool chargeLimitSupported() const { return m_chargeLimitSupported; }
    bool chargeLimitEnabled() const { return m_chargeLimitEnabled; }
    int chargeLimitStart() const { return m_chargeLimitStart; }
    int chargeLimitEnd() const { return m_chargeLimitEnd; }
    // False when UPower has the limit on but the kernel does not, for any of
    // the batteries: some battery controllers take the thresholds and reset
    // them a few seconds later.
    bool chargeLimitHeld() const;
    void setChargeLimitEnabled(bool enabled);

    cutefish::base::Signal<> readyChanged;
    cutefish::base::Signal<> availableChanged;
    cutefish::base::Signal<> batteriesChanged;
    cutefish::base::Signal<> capacityChanged;
    cutefish::base::Signal<> lastChargedChanged;
    cutefish::base::Signal<> udiChanged;
    cutefish::base::Signal<> chargeLimitChanged;

private:
    void watchBatteries();
    void updateChargeLimit();
    void readKernelChargeLimits();

    bool m_mock;
    std::shared_ptr<cutefish::upower::BatteryStatus> m_status;
    std::vector<std::string> m_batteries;
    // One for each of m_batteries.
    std::vector<std::unique_ptr<cutefish::upower::Device>> m_devices;
    std::vector<cutefish::base::Subscription> m_subscriptions;
    std::vector<cutefish::base::Subscription> m_deviceSubscriptions;
    cutefish::base::Timer m_recheck;
    int m_current = 0;

    bool m_chargeLimitSupported = false;
    bool m_chargeLimitEnabled = false;
    int m_chargeLimitStart = 0;
    int m_chargeLimitEnd = 0;
    // The end threshold the kernel reports for each of m_devices in
    // /sys/class/power_supply; -1 when it cannot be read.
    std::vector<int> m_kernelChargeLimits;
    int64_t m_mockLastCharged = 0;
};

} // namespace settings
