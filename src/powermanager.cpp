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

namespace
{
constexpr auto s_powerGroup = "Power";

int normalizedTimeout(int timeout)
{
    return timeout < 0 ? -1 : qMax(1, timeout);
}

int readTimeout(QSettings &settings, const QString &key, const QString &legacyKey, int fallback)
{
    settings.beginGroup(QLatin1String(s_powerGroup));
    const bool hasValue = settings.contains(key);
    const int value = hasValue ? settings.value(key).toInt() : fallback;
    settings.endGroup();

    if (hasValue)
        return normalizedTimeout(value);

    if (!legacyKey.isEmpty() && settings.contains(legacyKey))
        return normalizedTimeout(settings.value(legacyKey).toInt());

    return fallback;
}

bool readBool(QSettings &settings, const QString &key, bool fallback)
{
    settings.beginGroup(QLatin1String(s_powerGroup));
    const bool hasValue = settings.contains(key);
    const bool value = hasValue ? settings.value(key).toBool() : fallback;
    settings.endGroup();

    if (hasValue)
        return value;

    return settings.value(key, fallback).toBool();
}

void writePowerSetting(const QString &key, const QVariant &value)
{
    QSettings settings(QSettings::UserScope, "cutefishos", "power");
    settings.beginGroup(QLatin1String(s_powerGroup));
    settings.setValue(key, value);
    settings.endGroup();
    settings.sync();
}

void callPowerManager(const QString &method, const QVariant &value)
{
    QDBusInterface iface("com.cutefish.PowerManager",
                         "/PowerManager", "com.cutefish.PowerManager",
                         QDBusConnection::sessionBus());
    if (iface.isValid())
        iface.asyncCall(method, value);
}
}

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
    m_batteryScreenOff = readTimeout(settings, QStringLiteral("BatteryScreenOff"),
                                     QStringLiteral("CloseScreenTimeout"), 300);
    m_acScreenOff = readTimeout(settings, QStringLiteral("ACScreenOff"),
                                QStringLiteral("CloseScreenTimeout"), 1200);
    m_idleTime = m_acScreenOff;
    m_hibernateTime = settings.value("HibernateTimeout", 600).toInt();

    m_sleepWhenClosedScreen = readBool(settings, QStringLiteral("SleepWhenClosedScreen"), false);
    m_lockWhenClosedScreen = readBool(settings, QStringLiteral("LockWhenClosedScreen"), true);
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

int PowerManager::batteryScreenOff() const
{
    return m_batteryScreenOff;
}

void PowerManager::setBatteryScreenOff(int timeout)
{
    timeout = normalizedTimeout(timeout);
    if (m_batteryScreenOff == timeout)
        return;

    m_batteryScreenOff = timeout;
    writePowerSetting(QStringLiteral("BatteryScreenOff"), timeout);
    callPowerManager(QStringLiteral("setBatteryScreenOff"), timeout);
    emit batteryScreenOffChanged();
}

int PowerManager::acScreenOff() const
{
    return m_acScreenOff;
}

void PowerManager::setACScreenOff(int timeout)
{
    timeout = normalizedTimeout(timeout);
    if (m_acScreenOff == timeout)
        return;

    m_acScreenOff = timeout;
    m_idleTime = timeout;
    writePowerSetting(QStringLiteral("ACScreenOff"), timeout);
    callPowerManager(QStringLiteral("setACScreenOff"), timeout);
    emit acScreenOffChanged();
    emit idleTimeChanged();
}

int PowerManager::idleTime()
{
    return m_idleTime;
}

void PowerManager::setIdleTime(int idleTime)
{
    setBatteryScreenOff(idleTime);
    setACScreenOff(idleTime);
}

int PowerManager::hibernateTime()
{
    return m_hibernateTime;
}

void PowerManager::setHibernateTime(int timeout)
{
    if (m_hibernateTime != timeout) {
        m_hibernateTime = timeout;
        emit hibernateTimeChanged();
    }
}

bool PowerManager::sleepWhenClosedScreen() const
{
    return m_sleepWhenClosedScreen;
}

void PowerManager::setSleepWhenClosedScreen(bool sleepWhenClosedScreen)
{
    if (m_sleepWhenClosedScreen == sleepWhenClosedScreen)
        return;

    m_sleepWhenClosedScreen = sleepWhenClosedScreen;
    emit sleepWhenClosedScreenChanged();
    writePowerSetting(QStringLiteral("SleepWhenClosedScreen"), sleepWhenClosedScreen);
    callPowerManager(QStringLiteral("setSleepWhenClosedScreen"), sleepWhenClosedScreen);
}

bool PowerManager::lockWhenClosedScreen() const
{
    return m_lockWhenClosedScreen;
}

void PowerManager::setLockWhenClosedScreen(bool lockWhenClosedScreen)
{
    if (m_lockWhenClosedScreen == lockWhenClosedScreen)
        return;

    m_lockWhenClosedScreen = lockWhenClosedScreen;
    emit lockWhenClosedScreenChanged();
    writePowerSetting(QStringLiteral("LockWhenClosedScreen"), lockWhenClosedScreen);
    callPowerManager(QStringLiteral("setLockWhenClosedScreen"), lockWhenClosedScreen);
}
