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

#include <QDBusInterface>
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
    if (m_iface.isValid())
        m_mode = m_iface.property("mode").toInt();

    QSettings settings(QSettings::UserScope, "cutefishos", "power");
    m_batteryScreenOff = readTimeout(settings, QStringLiteral("BatteryScreenOff"),
                                     QStringLiteral("CloseScreenTimeout"), 300);
    m_acScreenOff = readTimeout(settings, QStringLiteral("ACScreenOff"),
                                QStringLiteral("CloseScreenTimeout"), 1200);
}

int PowerManager::mode() const
{
    return m_mode;
}

void PowerManager::setMode(int mode)
{
    if (m_mode == mode)
        return;

    if (m_iface.isValid())
        m_iface.asyncCall(QStringLiteral("setMode"), mode);

    m_mode = mode;
    emit modeChanged();
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
    writePowerSetting(QStringLiteral("ACScreenOff"), timeout);
    callPowerManager(QStringLiteral("setACScreenOff"), timeout);
    emit acScreenOffChanged();
}
