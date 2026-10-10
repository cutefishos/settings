// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "time.h"
#include "timedated_interface.h"

#include <QDBusPendingCallWatcher>
#include <QDebug>
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

    QDBusConnection::systemBus().connect(QStringLiteral("org.freedesktop.timedate1"),
                                         QStringLiteral("/org/freedesktop/timedate1"),
                                         QStringLiteral("org.freedesktop.DBus.Properties"),
                                         QStringLiteral("PropertiesChanged"), this,
                                         SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
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
        // timedated only asks the polkit agent when the message itself allows it.
        iface.setInteractiveAuthorizationAllowed(true);
        // The reply waits for the password; PropertiesChanged still corrects a late answer.
        iface.setTimeout(120000);
        auto *watcher = new QDBusPendingCallWatcher(iface.SetNTP(m_useNtp, true), this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, enabled](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<> reply = *watcher;
            watcher->deleteLater();

            // A cancelled or failed polkit prompt leaves timedated as it was.
            if (reply.isError() && m_useNtp == enabled) {
                qWarning() << "Failed to change automatic time sync" << reply.error().message();
                m_useNtp = !enabled;
                emit useNtpChanged();
            }
        });

        emit useNtpChanged();
    }
}

void Time::onPropertiesChanged(const QString &interface, const QVariantMap &changed,
                               const QStringList &invalidated)
{
    Q_UNUSED(invalidated);

    if (interface != QLatin1String("org.freedesktop.timedate1") || !changed.contains(QStringLiteral("NTP")))
        return;

    const bool useNtp = changed.value(QStringLiteral("NTP")).toBool();
    if (m_useNtp != useNtp) {
        m_useNtp = useNtp;
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
