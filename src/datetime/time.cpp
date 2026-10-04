// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "time.h"
#include "timedated_interface.h"

#include <QSettings>

#include <preferences.h>

Time::Time(QObject *parent)
    : QObject(parent)
{
    QSettings settings(Preferences::file(QStringLiteral("shell")), QSettings::IniFormat);
    m_twentyFour = settings.value("Statusbar/TwentyFourTime", false).toBool();

    OrgFreedesktopTimedate1Interface iface(QStringLiteral("org.freedesktop.timedate1"),
                                           QStringLiteral("/org/freedesktop/timedate1"),
                                           QDBusConnection::systemBus());

    m_canNTP = iface.canNTP();
    m_useNtp = iface.nTP();
}

bool Time::useNtp() const
{
    return m_useNtp;
}

bool Time::canNTP() const
{
    return m_canNTP;
}

void Time::setUseNtp(bool enabled)
{
    if (m_useNtp != enabled) {
        m_useNtp = enabled;

        OrgFreedesktopTimedate1Interface iface(QStringLiteral("org.freedesktop.timedate1"),
                                               QStringLiteral("/org/freedesktop/timedate1"),
                                               QDBusConnection::systemBus());
        iface.SetNTP(m_useNtp, true);

        emit useNtpChanged();
    }
}

bool Time::twentyFour() const
{
    return m_twentyFour;
}

void Time::setTwentyFour(bool t)
{
    if (m_twentyFour != t) {
        m_twentyFour = t;
        QDBusInterface("com.cutefish.Statusbar", "/Statusbar")
                .call("setTwentyFourTime", m_twentyFour);

        emit twentyFourChanged();
    }
}
