// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "powermanager.h"

#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusVariant>
#include <QSettings>

#include <preferences.h>

namespace {
constexpr auto s_powerGroup = "Power";

const QString s_profilesService = QStringLiteral("org.freedesktop.UPower.PowerProfiles");
const QString s_profilesPath = QStringLiteral("/org/freedesktop/UPower/PowerProfiles");
const QString s_propertiesInterface = QStringLiteral("org.freedesktop.DBus.Properties");

int normalizedTimeout(int timeout)
{
    return timeout < 0 ? -1 : qMax(1, timeout);
}

int readTimeout(QSettings &settings, const QString &key, int fallback)
{
    return settings.contains(key) ? normalizedTimeout(settings.value(key).toInt()) : fallback;
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

    QSettings settings(Preferences::file(QStringLiteral("services")), QSettings::IniFormat);
    settings.beginGroup(QLatin1String(s_powerGroup));
    m_batteryScreenOff = readTimeout(settings, QStringLiteral("BatteryScreenOff"), 300);
    m_acScreenOff = readTimeout(settings, QStringLiteral("ACScreenOff"), 1200);
    m_acPreventSleep = settings.value(QStringLiteral("ACPreventSleep"), false).toBool();
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
    callPowerManager(QStringLiteral("setACPreventSleep"), prevent);
    emit acPreventSleepChanged();
}
