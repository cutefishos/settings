// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/datetime/time.h"

#include "base/paths.h"
#include "cutefish/timedate/time_date.h"
#include "pages/language/region.h"

using namespace cutefish;
using namespace cutefish::base;

namespace settings {

std::string TimeSettings::defaultConfigPath()
{
    return paths::cutefishConfig("services");
}

TimeSettings::TimeSettings(timedate::TimeDate& timeDate, std::string configPath)
    : m_timeDate(timeDate)
    , m_configPath(std::move(configPath))
    , m_service(dbus::Connection::session(), "org.cutefish.Services", "/org/cutefish/Services/Language",
                "org.cutefish.Services.Language")
    , m_canNtp(timeDate.canNtp())
    , m_useNtp(timeDate.ntp())
{
    m_twentyFour = Region::sessionLocale(m_configPath).usesTwentyFourHourTime();
    // A change made elsewhere, or the region's clock after a new region.
    m_changedMatch = m_service.onSignal("languageChanged", [this](const dbus::Message&) {
        const bool twentyFour = Region::sessionLocale(m_configPath).usesTwentyFourHourTime();
        if (twentyFour != m_twentyFour) {
            m_twentyFour = twentyFour;
            twentyFourChanged.emit();
        }
    });
    m_timeDateChanged = m_timeDate.changed.connect([this] {
        if (m_timeDate.ntp() != m_useNtp) {
            m_useNtp = m_timeDate.ntp();
            useNtpChanged.emit();
        }
    });
}

void TimeSettings::setUseNtp(bool enabled)
{
    if (m_useNtp == enabled)
        return;
    m_useNtp = enabled;
    m_timeDate.setNtp(enabled, [this, enabled](const std::string& error) {
        // A cancelled or failed polkit prompt leaves timedated as it was.
        if (!error.empty() && m_useNtp == enabled) {
            m_useNtp = !enabled;
            useNtpChanged.emit();
        }
    });
    useNtpChanged.emit();
}

void TimeSettings::setTwentyFour(bool enabled)
{
    if (m_twentyFour == enabled)
        return;
    m_twentyFour = enabled;
    m_service.send("setTwentyFourHour", int32_t(enabled ? 2 : 1));
    twentyFourChanged.emit();
}

} // namespace settings
