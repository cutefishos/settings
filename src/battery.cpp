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

#include "battery.h"
#include <QCoreApplication>
#include <QDBusArgument>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDateTime>
#include <QDebug>
#include <QLocale>
#include <QSettings>

static const QString s_sServer = "com.cutefish.Services";
static const QString s_sPath = "/com/cutefish/Services/Battery";
static const QString s_sInterface = "com.cutefish.Services.Battery";

static bool mockBatteryEnabled()
{
    return QCoreApplication::arguments().contains(QStringLiteral("--mock-battery"));
}

// DBus Battery Info Structure
struct BatteryInfo {
    uint time, state;
    double value;
};
Q_DECLARE_METATYPE(BatteryInfo)

const QDBusArgument &operator<<(QDBusArgument &argument, const BatteryInfo &info)
{
    argument.beginStructure();
    argument << info.time << info.value << info.state;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, BatteryInfo &info)
{
    argument.beginStructure();
    argument >> info.time >> info.value >> info.state;
    argument.endStructure();
    return argument;
}

Battery::Battery(QObject *parent)
    : QObject(parent)
    , m_upowerInterface("org.freedesktop.UPower", "/org/freedesktop/UPower",
                        "org.freedesktop.UPower", QDBusConnection::systemBus())
    , m_interface("com.cutefish.Services", "/com/cutefish/Services/Battery",
                  "com.cutefish.Services.Battery", QDBusConnection::sessionBus())
    , m_available(false)
    , m_onBattery(false)
{
    if (mockBatteryEnabled()) {
        m_available = true;
        m_onBattery = true;
        m_udi = QStringLiteral("mock-battery");
        m_chargeLimitSupported = true;
        m_chargeLimitStart = 75;
        m_chargeLimitEnd = 80;
        return;
    }

    m_available = m_interface.isValid() && !m_interface.lastError().isValid();

    if (m_available) {
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface,
                                              "chargeStateChanged", this,
                                              SLOT(chargeStateChanged(int)));
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface,
                                              "chargePercentChanged", this,
                                              SLOT(chargePercentChanged(int)));
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface,
                                              "lastChargedPercentChanged", this,
                                              SLOT(lastChargedPercentChanged()));
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface, "capacityChanged",
                                              this, SLOT(capacityChanged(int)));
        QDBusConnection::sessionBus().connect(s_sServer, s_sPath, s_sInterface,
                                              "remainingTimeChanged", this,
                                              SLOT(remainingTimeChanged(qlonglong)));

        QDBusInterface interface("org.freedesktop.UPower", "/org/freedesktop/UPower",
                                 "org.freedesktop.UPower", QDBusConnection::systemBus());

        QDBusConnection::systemBus().connect(
                "org.freedesktop.UPower", "/org/freedesktop/UPower",
                "org.freedesktop.DBus.Properties", "PropertiesChanged", this,
                SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));

        if (interface.isValid()) {
            m_onBattery = interface.property("OnBattery").toBool();
        }

        m_udi = m_interface.property("udi").toString();

        if (!m_udi.isEmpty()) {
            QDBusConnection::systemBus().connect(
                    "org.freedesktop.UPower", m_udi, "org.freedesktop.DBus.Properties",
                    "PropertiesChanged", this,
                    SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));
            loadChargeLimit();
        }

        emit udiChanged();
        emit validChanged();
    }
}

bool Battery::available() const
{
    return m_available;
}

bool Battery::onBattery() const
{
    return m_onBattery;
}

void Battery::refresh()
{
    if (mockBatteryEnabled())
        return;

    m_interface.call("refresh");
}

QVariantList Battery::getHistory(const int timespan, const int resolution)
{
    QVariantList listValues;
    QVariantMap listItem;

    QDBusMessage historyMessage = QDBusMessage::createMethodCall(
            "org.freedesktop.UPower", m_udi, "org.freedesktop.UPower.Device", "GetHistory");
    QVariantList historyMessageArguments;
    historyMessageArguments.append("charge");
    historyMessageArguments.append((uint)timespan);
    historyMessageArguments.append((uint)resolution);
    historyMessage.setArguments(historyMessageArguments);

    quint64 offset = QDateTime::currentDateTime().toSecsSinceEpoch();

    QDBusReply<QDBusArgument> historyArgument = QDBusConnection::systemBus().call(historyMessage);
    if (historyArgument.isValid()) {
        QDBusArgument arrayArgument = historyArgument.value();
        arrayArgument.beginArray();
        while (!arrayArgument.atEnd()) {
            BatteryInfo info;
            arrayArgument >> info;
            listItem.insert("time", offset - info.time);
            listItem.insert("value", info.value);

            if (info.value > 0)
                listValues += listItem;
        }
    }

    return listValues;
}

int Battery::chargeState() const
{
    if (mockBatteryEnabled())
        return 2;

    return m_interface.property("chargeState").toInt();
}

int Battery::chargePercent() const
{
    if (mockBatteryEnabled())
        return 78;

    return m_interface.property("chargePercent").toInt();
}

int Battery::lastChargedPercent() const
{
    if (mockBatteryEnabled())
        return 100;

    return m_interface.property("lastChargedPercent").toInt();
}

int Battery::capacity() const
{
    if (mockBatteryEnabled())
        return 94;

    return m_interface.property("capacity").toInt();
}

QString Battery::statusString() const
{
    if (mockBatteryEnabled())
        return tr("Discharging · 3 hours 24 minutes remaining");

    return m_interface.property("statusString").toString();
}

QString Battery::lastChargedTime() const
{
    const qint64 secs = mockBatteryEnabled() ? QDateTime::currentSecsSinceEpoch() - 2 * 3600
                                             : m_interface.property("lastChargedSecs").toLongLong();

    if (secs <= 0)
        return QString();

    // A date and time, as the file manager shows them; the service's own
    // "13 h 15 min ago" is worked out once and goes stale on an open page.
    const QDateTime time = QDateTime::fromSecsSinceEpoch(secs);
    const QLocale locale;
    const QString clock = locale.toString(time.time(), QLocale::ShortFormat);
    const qint64 days = time.date().daysTo(QDate::currentDate());

    if (days == 0)
        return tr("Today at %1").arg(clock);
    if (days == 1)
        return tr("Yesterday at %1").arg(clock);

    return locale.toString(time, QLocale::ShortFormat);
}

bool Battery::showPercent()
{
    if (mockBatteryEnabled())
        return true;

    QSettings settings(QSettings::UserScope, "cutefish", "battery");
    return settings.value("BatteryPercentage", false).toBool();
}

void Battery::setPercentEnabled(bool value)
{
    if (mockBatteryEnabled())
        return;

    QDBusInterface("com.cutefish.Statusbar", "/Statusbar").call("setBatteryPercentage", value);
}

QString Battery::udi() const
{
    return m_udi;
}

bool Battery::chargeLimitSupported() const
{
    return m_chargeLimitSupported;
}

bool Battery::chargeLimitEnabled() const
{
    return m_chargeLimitEnabled;
}

int Battery::chargeLimitStart() const
{
    return m_chargeLimitStart;
}

int Battery::chargeLimitEnd() const
{
    return m_chargeLimitEnd;
}

void Battery::setChargeLimitEnabled(bool enabled)
{
    if (mockBatteryEnabled()) {
        m_chargeLimitEnabled = enabled;
        emit chargeLimitChanged();
        return;
    }

    QDBusMessage message = QDBusMessage::createMethodCall(
            "org.freedesktop.UPower", m_udi, "org.freedesktop.UPower.Device", "EnableChargeThreshold");
    message << enabled;
    // Polkit lets the active session through without asking; anyone else gets a prompt.
    message.setInteractiveAuthorizationAllowed(true);

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<> reply = *watcher;
        watcher->deleteLater();

        if (reply.isError()) {
            qWarning() << "Failed to change the charge limit" << reply.error().message();
            // Puts a switch bound to chargeLimitEnabled back.
            emit chargeLimitChanged();
        }
    });
}

void Battery::loadChargeLimit()
{
    QDBusMessage message = QDBusMessage::createMethodCall(
            "org.freedesktop.UPower", m_udi, "org.freedesktop.DBus.Properties", "GetAll");
    message << QStringLiteral("org.freedesktop.UPower.Device");

    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *watcher) {
        QDBusPendingReply<QVariantMap> reply = *watcher;
        watcher->deleteLater();

        if (!reply.isError())
            updateChargeLimit(reply.value());
    });
}

// UPower before 1.90 has none of these properties, which leaves the limit unsupported.
void Battery::updateChargeLimit(const QVariantMap &props)
{
    const bool supported = props.value("ChargeThresholdSupported", m_chargeLimitSupported).toBool();
    const bool enabled = props.value("ChargeThresholdEnabled", m_chargeLimitEnabled).toBool();
    const int start = props.value("ChargeStartThreshold", m_chargeLimitStart).toInt();
    const int end = props.value("ChargeEndThreshold", m_chargeLimitEnd).toInt();

    if (supported == m_chargeLimitSupported && enabled == m_chargeLimitEnabled
        && start == m_chargeLimitStart && end == m_chargeLimitEnd)
        return;

    m_chargeLimitSupported = supported;
    m_chargeLimitEnabled = enabled;
    m_chargeLimitStart = start;
    m_chargeLimitEnd = end;
    emit chargeLimitChanged();
}

void Battery::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps,
                                  const QStringList &invalidatedProps)
{
    Q_UNUSED(ifaceName);
    Q_UNUSED(changedProps);
    Q_UNUSED(invalidatedProps);

    bool onBattery = m_upowerInterface.property("OnBattery").toBool();
    if (onBattery != m_onBattery) {
        m_onBattery = onBattery;
        m_interface.call("refresh");
        emit onBatteryChanged();
    }
}

void Battery::onDevicePropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps,
                                        const QStringList &invalidatedProps)
{
    Q_UNUSED(invalidatedProps);

    if (ifaceName == QLatin1String("org.freedesktop.UPower.Device"))
        updateChargeLimit(changedProps);
}
