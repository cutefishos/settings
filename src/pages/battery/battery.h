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

// The battery page's view of cutefish-services' battery, plus UPower's
// charge limit. With --mock-battery it shows sample values instead.
class Battery
{
public:
    Battery();
    ~Battery();

    Battery(const Battery&) = delete;
    Battery& operator=(const Battery&) = delete;

    bool isReady() const;
    bool isAvailable() const;

    int lastChargedPercent() const;
    int capacity() const;
    // "Today at 2:21 pm", or the date and time; empty when never charged.
    std::string lastChargedTime() const;

    // Whether the status bar shows the percentage.
    bool showPercent() const;
    void setPercentEnabled(bool enabled);

    // UPower's charge limit: only on or off, between thresholds the system sets.
    bool chargeLimitSupported() const { return m_chargeLimitSupported; }
    bool chargeLimitEnabled() const { return m_chargeLimitEnabled; }
    int chargeLimitStart() const { return m_chargeLimitStart; }
    int chargeLimitEnd() const { return m_chargeLimitEnd; }
    // False when UPower has the limit on but the kernel does not: some battery
    // controllers take the thresholds and reset them a few seconds later.
    bool chargeLimitHeld() const;
    void setChargeLimitEnabled(bool enabled);

    // The UPower device object path.
    const std::string& udi() const { return m_udi; }

    cutefish::base::Signal<> readyChanged;
    cutefish::base::Signal<> availableChanged;
    cutefish::base::Signal<> capacityChanged;
    cutefish::base::Signal<> lastChargedChanged;
    cutefish::base::Signal<> udiChanged;
    cutefish::base::Signal<> chargeLimitChanged;

private:
    void watchDevice();
    void updateChargeLimit(const cutefish::base::dbus::VariantMap& properties);
    void readKernelChargeLimit();

    bool m_mock;
    std::shared_ptr<cutefish::upower::BatteryStatus> m_status;
    std::unique_ptr<cutefish::upower::Device> m_device;
    std::vector<cutefish::base::Subscription> m_subscriptions;
    cutefish::base::Subscription m_deviceSubscription;
    cutefish::base::Timer m_recheck;
    std::string m_udi;

    bool m_chargeLimitSupported = false;
    bool m_chargeLimitEnabled = false;
    int m_chargeLimitStart = 0;
    int m_chargeLimitEnd = 0;
    // The battery's directory in /sys/class/power_supply, and the end
    // threshold the kernel reports there; -1 when it cannot be read.
    std::string m_batteryPath;
    int m_kernelChargeLimit = -1;
    int64_t m_mockLastCharged = 0;
};

} // namespace settings
