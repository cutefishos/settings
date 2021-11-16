/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#include "powermanager.h"

#include <QSettings>
#include <QDBusPendingCall>
#include <QDebug>

PowerManager::PowerManager(QObject *parent)
    : QObject(parent)
    , m_iface("com.cutefish.PowerManager",
              "/CPUManagement", "com.cutefish.CPUManagement",
              QDBusConnection::sessionBus())
    , m_mode(-1)
{
    if (m_iface.isValid()) {
        m_mode = m_iface.property("mode").toInt();
    }

    QSettings settings(QSettings::UserScope, "cutefishos", "power");
    m_idleTime = settings.value("CloseScreenTimeout", 600).toInt();
    m_hibernateTime = settings.value("HibernateTimeout", 600).toInt();
}

int PowerManager::mode() const
{
    return m_mode;
}

void PowerManager::setMode(int mode)
{
    if (m_mode != mode) {
        m_iface.asyncCall("setMode", mode);
        m_mode = mode;
        emit modeChanged();
    }
}

int PowerManager::idleTime()
{
    return m_idleTime;
}

void PowerManager::setIdleTime(int idleTime)
{
    if (m_idleTime != idleTime) {
        m_idleTime = idleTime;
        QSettings settings(QSettings::UserScope, "cutefishos", "power");
        settings.setValue("CloseScreenTimeout", idleTime);
        emit idleTimeChanged();
    }
}

int PowerManager::hibernateTime()
{
    return m_hibernateTime;
}

void PowerManager::setHibernateTime(int timeout)
{
    if (m_hibernateTime != timeout) {
        m_hibernateTime = timeout;
        QSettings settings(QSettings::UserScope, "cutefishos", "power");
        settings.setValue("HibernateTimeout", timeout);
        emit hibernateTimeChanged();
    }
}
