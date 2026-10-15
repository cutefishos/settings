// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "base/dbus.h"
#include "base/signal.h"

namespace cutefish::timedate {
class TimeDate;
}

namespace settings {

// Network time, through timedated, and the status bar's 24-hour clock.
class TimeSettings
{
public:
    explicit TimeSettings(cutefish::timedate::TimeDate& timeDate);

    bool useNtp() const { return m_useNtp; }
    // Shows at once; a refused or cancelled password puts it back.
    void setUseNtp(bool enabled);
    // Whether a network time service is installed.
    bool canNtp() const { return m_canNtp; }

    bool twentyFour() const { return m_twentyFour; }
    void setTwentyFour(bool enabled);

    cutefish::base::Signal<> useNtpChanged;
    cutefish::base::Signal<> twentyFourChanged;

private:
    cutefish::timedate::TimeDate& m_timeDate;
    cutefish::base::Subscription m_timeDateChanged;
    cutefish::base::dbus::Proxy m_statusBar;
    bool m_canNtp;
    bool m_useNtp;
    bool m_twentyFour;
};

} // namespace settings
