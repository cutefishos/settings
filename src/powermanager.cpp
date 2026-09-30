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

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QSettings>

namespace {
constexpr auto s_powerGroup = "Power";

const QString s_profilesService = QStringLiteral("org.freedesktop.UPower.PowerProfiles");
const QString s_profilesPath = QStringLiteral("/org/freedesktop/UPower/PowerProfiles");
const QString s_propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");

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
    QSettings settings(QSettings::UserScope, "cutefish", "power");
    settings.beginGroup(QLatin1String(s_powerGroup));
    settings.setValue(key, value);
    settings.endGroup();
    settings.sync();
}

void callPowerManager(const QString &method, const QVariant &value)
{
    QDBusInterface iface("com.cutefish.Services", "/com/cutefish/Services/Power",
                         "com.cutefish.Services.Power", QDBusConnection::sessionBus());
    if (iface.isValid())
        iface.asyncCall(method, value);
}

} // namespace

PowerManager::PowerManager(QObject *parent)
    : QObject(parent)
{
    QDBusConnection::systemBus().connect(s_profilesService, s_profilesPath, s_propertiesInterface,
                                         QStringLiteral("PropertiesChanged"), this,
                                         SLOT(onProfilePropertiesChanged(QString, QVariantMap, QStringList)));
    loadProfileProperties();

    QSettings settings(QSettings::UserScope, "cutefish", "power");
    m_batteryScreenOff = readTimeout(settings, QStringLiteral("BatteryScreenOff"),
                                     QStringLiteral("CloseScreenTimeout"), 300);
    m_acScreenOff = readTimeout(settings, QStringLiteral("ACScreenOff"),
                                QStringLiteral("CloseScreenTimeout"), 1200);
    settings.beginGroup(QLatin1String(s_powerGroup));
    m_acPreventSleep = settings.value(QStringLiteral("ACPreventSleep"), false).toBool();
    settings.endGroup();
}

QString PowerManager::profile() const
{
    return m_profile;
}

void PowerManager::setProfile(const QString &profile)
{
    if (m_profile == profile || !m_profiles.contains(profile))
        return;

    QDBusMessage message = QDBusMessage::createMethodCall(s_profilesService, s_profilesPath,
                                                          s_propertiesInterface, QStringLiteral("Set"));
    message << s_profilesService << QStringLiteral("ActiveProfile")
            << QVariant::fromValue(QDBusVariant(profile));

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        // A refused switch (e.g. by polkit) sends no PropertiesChanged; re-read the real profile.
        if (watcher->isError())
            loadProfileProperties();

        watcher->deleteLater();
    });

    m_profile = profile;
    emit profileChanged();
}

QStringList PowerManager::profiles() const
{
    return m_profiles;
}

QString PowerManager::performanceDegraded() const
{
    return m_performanceDegraded;
}

void PowerManager::onProfilePropertiesChanged(const QString &interface, const QVariantMap &changed,
                                              const QStringList &invalidated)
{
    Q_UNUSED(invalidated)

    if (interface == s_profilesService)
        updateProfileProperties(changed);
}

void PowerManager::loadProfileProperties()
{
    QDBusMessage message = QDBusMessage::createMethodCall(s_profilesService, s_profilesPath,
                                                          s_propertiesInterface, QStringLiteral("GetAll"));
    message << s_profilesService;

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QVariantMap> reply = *watcher;
        if (!reply.isError())
            updateProfileProperties(reply.value());

        watcher->deleteLater();
    });
}

void PowerManager::updateProfileProperties(const QVariantMap &properties)
{
    auto it = properties.constFind(QStringLiteral("Profiles"));
    if (it != properties.constEnd()) {
        QStringList profiles;
        const QDBusArgument argument = it->value<QDBusArgument>();

        argument.beginArray();
        while (!argument.atEnd()) {
            QVariantMap profile;
            argument >> profile;
            profiles.append(profile.value(QStringLiteral("Profile")).toString());
        }
        argument.endArray();

        if (m_profiles != profiles) {
            m_profiles = profiles;
            emit profilesChanged();
        }
    }

    it = properties.constFind(QStringLiteral("ActiveProfile"));
    if (it != properties.constEnd() && m_profile != it->toString()) {
        m_profile = it->toString();
        emit profileChanged();
    }

    it = properties.constFind(QStringLiteral("PerformanceDegraded"));
    if (it != properties.constEnd() && m_performanceDegraded != it->toString()) {
        m_performanceDegraded = it->toString();
        emit performanceDegradedChanged();
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
    writePowerSetting(QStringLiteral("ACScreenOff"), timeout);
    callPowerManager(QStringLiteral("setACScreenOff"), timeout);
    emit acScreenOffChanged();
}

bool PowerManager::acPreventSleep() const
{
    return m_acPreventSleep;
}

void PowerManager::setACPreventSleep(bool prevent)
{
    if (m_acPreventSleep == prevent)
        return;

    m_acPreventSleep = prevent;
    writePowerSetting(QStringLiteral("ACPreventSleep"), prevent);
    callPowerManager(QStringLiteral("setACPreventSleep"), prevent);
    emit acPreventSleepChanged();
}
