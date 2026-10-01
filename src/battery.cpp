// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "battery.h"
#include <QCoreApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QLocale>
#include <QSettings>
#include <QTimer>

#include <batterystatus.h>
#include <preferences.h>

static bool mockBatteryEnabled()
{
    return QCoreApplication::arguments().contains(QStringLiteral("--mock-battery"));
}

Battery::Battery(QObject *parent)
    : QObject(parent)
    , m_status(BatteryStatus::instance())
{
    if (mockBatteryEnabled()) {
        m_udi = QStringLiteral("mock-battery");
        m_chargeLimitSupported = true;
        m_chargeLimitStart = 75;
        m_chargeLimitEnd = 80;
        return;
    }

    connect(m_status, &BatteryStatus::readyChanged, this, &Battery::readyChanged);
    connect(m_status, &BatteryStatus::availableChanged, this, &Battery::validChanged);
    connect(m_status, &BatteryStatus::capacityChanged, this, &Battery::capacityChanged);
    connect(m_status, &BatteryStatus::lastChargedChanged, this,
            &Battery::lastChargedPercentChanged);
    connect(m_status, &BatteryStatus::udiChanged, this, &Battery::watchDevice);

    watchDevice();
}

void Battery::watchDevice()
{
    if (m_status->udi() == m_udi)
        return;

    QDBusConnection bus = QDBusConnection::systemBus();
    if (!m_udi.isEmpty())
        bus.disconnect("org.freedesktop.UPower", m_udi, "org.freedesktop.DBus.Properties",
                       "PropertiesChanged", this,
                       SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));

    m_udi = m_status->udi();

    if (!m_udi.isEmpty()) {
        bus.connect("org.freedesktop.UPower", m_udi, "org.freedesktop.DBus.Properties",
                    "PropertiesChanged", this,
                    SLOT(onDevicePropertiesChanged(QString, QVariantMap, QStringList)));
        loadChargeLimit();
    }

    emit udiChanged();
}

bool Battery::ready() const
{
    return mockBatteryEnabled() || m_status->ready();
}

bool Battery::available() const
{
    return mockBatteryEnabled() || m_status->available();
}

int Battery::lastChargedPercent() const
{
    if (mockBatteryEnabled())
        return 100;

    return m_status->lastChargedPercent();
}

int Battery::capacity() const
{
    if (mockBatteryEnabled())
        return 94;

    return m_status->capacity();
}

QString Battery::lastChargedTime() const
{
    const qint64 secs = mockBatteryEnabled() ? QDateTime::currentSecsSinceEpoch() - 2 * 3600
                                             : m_status->lastChargedSecs();

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

    QSettings settings(Preferences::file(QStringLiteral("shell")), QSettings::IniFormat);
    return settings.value("Statusbar/BatteryPercentage", false).toBool();
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

bool Battery::chargeLimitHeld() const
{
    if (!m_chargeLimitEnabled || m_kernelChargeLimit < 0)
        return true;

    return m_kernelChargeLimit == m_chargeLimitEnd;
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

        if (reply.isError())
            return;

        const QString nativePath = reply.value().value("NativePath").toString();

        if (!nativePath.isEmpty())
            m_batteryPath = QStringLiteral("/sys/class/power_supply/") + nativePath + QLatin1Char('/');

        updateChargeLimit(reply.value());
        readKernelChargeLimit();
    });
}

void Battery::readKernelChargeLimit()
{
    if (m_batteryPath.isEmpty())
        return;

    QFile file(m_batteryPath + QStringLiteral("charge_control_end_threshold"));
    const int limit = file.open(QIODevice::ReadOnly) ? file.readAll().trimmed().toInt() : -1;

    if (limit == m_kernelChargeLimit)
        return;

    m_kernelChargeLimit = limit;
    emit chargeLimitChanged();
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

    const bool turnedOn = enabled && !m_chargeLimitEnabled;

    m_chargeLimitSupported = supported;
    m_chargeLimitEnabled = enabled;
    m_chargeLimitStart = start;
    m_chargeLimitEnd = end;
    emit chargeLimitChanged();

    // A controller that rejects the thresholds resets them within seconds,
    // without UPower or the kernel announcing it.
    if (turnedOn)
        QTimer::singleShot(5000, this, &Battery::readKernelChargeLimit);
}

void Battery::onDevicePropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps,
                                        const QStringList &invalidatedProps)
{
    Q_UNUSED(invalidatedProps);

    if (ifaceName != QLatin1String("org.freedesktop.UPower.Device"))
        return;

    updateChargeLimit(changedProps);
    // UPower updates the battery every little while; a cheap moment to look again.
    readKernelChargeLimit();
}
