/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Kate Leet <kateleet@cutefishos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "notifications.h"

#include <QDBusMetaType>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>

Notifications::Notifications(QObject *parent)
    : QObject(parent)
    , m_iface("com.cutefish.Notification",
              "/Notification",
              "com.cutefish.Notification", QDBusConnection::sessionBus())
{
    m_doNotDisturb = m_iface.property("doNotDisturb").toBool();

    QDBusConnection::sessionBus().connect("com.cutefish.Notification",
                                          "/Notification",
                                          "com.cutefish.Notification",
                                          "doNotDisturbChanged", this, SLOT(onDBusDoNotDisturbChanged()));

    qDBusRegisterMetaType<QList<QVariantMap>>();
    QDBusConnection::sessionBus().connect("com.cutefish.Notification",
                                          "/Notification",
                                          "com.cutefish.Notification",
                                          "applicationsChanged", this, SLOT(loadApplications()));
    loadApplications();
}

bool Notifications::doNotDisturb() const
{
    return m_doNotDisturb;
}

void Notifications::setDoNotDisturb(bool enabled)
{
    m_doNotDisturb = enabled;

    QDBusInterface iface("com.cutefish.Notification",
                         "/Notification",
                         "com.cutefish.Notification", QDBusConnection::sessionBus());

    if (iface.isValid()) {
        iface.asyncCall("setDoNotDisturb", enabled);
    }

    emit doNotDisturbChanged();
}

void Notifications::onDBusDoNotDisturbChanged()
{
    m_doNotDisturb = m_iface.property("doNotDisturb").toBool();
    emit doNotDisturbChanged();
}

QVariantList Notifications::applications() const
{
    return m_applications;
}

// The list is not updated here: the shell's applicationsChanged reloads it.
void Notifications::setApplicationMode(const QString &key, int mode)
{
    m_iface.asyncCall(QStringLiteral("setApplicationMode"), key, mode);
}

void Notifications::loadApplications()
{
    QDBusPendingCall call = m_iface.asyncCall(QStringLiteral("applications"));
    auto *watcher = new QDBusPendingCallWatcher(call, this);

    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QList<QVariantMap>> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError())
            return;

        m_applications.clear();
        for (const QVariantMap &application : reply.value())
            m_applications.append(application);

        emit applicationsChanged();
    });
}
