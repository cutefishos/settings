// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>

#include "base/dbus.h"
#include "base/signal.h"

namespace cutefish::timedate {
class TimeDate;
}

namespace settings {

// Network time, through timedated, and the session's 24-hour clock, which
// cutefish-services keeps with the region.
class TimeSettings
{
public:
    // `configPath` is cutefish-services' settings.
    explicit TimeSettings(cutefish::timedate::TimeDate& timeDate, std::string configPath = defaultConfigPath());

    bool useNtp() const { return m_useNtp; }
    // Shows at once; a refused or cancelled password puts it back.
    void setUseNtp(bool enabled);
    // Whether a network time service is installed.
    bool canNtp() const { return m_canNtp; }

    // The choice made, else the region's clock.
    bool twentyFour() const { return m_twentyFour; }
    void setTwentyFour(bool enabled);

    static std::string defaultConfigPath();

    cutefish::base::Signal<> useNtpChanged;
    cutefish::base::Signal<> twentyFourChanged;

private:
    cutefish::timedate::TimeDate& m_timeDate;
    cutefish::base::Subscription m_timeDateChanged;
    std::string m_configPath;
    cutefish::base::dbus::Proxy m_service;
    cutefish::base::dbus::Match m_changedMatch;
    bool m_canNtp;
    bool m_useNtp;
    bool m_twentyFour;
};

} // namespace settings
