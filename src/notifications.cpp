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
#include <QSettings>
#include <QDBusInterface>
#include <QDBusPendingCall>

Notifications::Notifications(QObject *parent)
    : QObject(parent)
{
    QSettings settings(QSettings::UserScope, "cutefishos", "notification");
    m_doNotDisturb = settings.value("DoNotDisturb", false).toBool();
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
