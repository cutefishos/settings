// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BATTERY_H
#define BATTERY_H

#include <QObject>
#include <QVariantMap>

class BatteryStatus;

// The battery page's view of cutefish-services' battery, plus UPower's charge limit.
class Battery : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)
    Q_PROPERTY(bool available READ available NOTIFY validChanged)
    Q_PROPERTY(int chargeState READ chargeState NOTIFY chargeStateChanged)
    Q_PROPERTY(int chargePercent READ chargePercent NOTIFY chargePercentChanged)
    Q_PROPERTY(int lastChargedPercent READ lastChargedPercent NOTIFY lastChargedPercentChanged)
    Q_PROPERTY(QString lastChargedTime READ lastChargedTime NOTIFY lastChargedPercentChanged)
    Q_PROPERTY(int capacity READ capacity NOTIFY capacityChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY remainingTimeChanged)
    Q_PROPERTY(bool onBattery READ onBattery NOTIFY onBatteryChanged)
    Q_PROPERTY(QString udi READ udi NOTIFY udiChanged)
    Q_PROPERTY(bool showPercent READ showPercent CONSTANT)
    Q_PROPERTY(bool chargeLimitSupported READ chargeLimitSupported NOTIFY chargeLimitChanged)
    Q_PROPERTY(bool chargeLimitEnabled READ chargeLimitEnabled NOTIFY chargeLimitChanged)
    Q_PROPERTY(int chargeLimitStart READ chargeLimitStart NOTIFY chargeLimitChanged)
    Q_PROPERTY(int chargeLimitEnd READ chargeLimitEnd NOTIFY chargeLimitChanged)
    Q_PROPERTY(bool chargeLimitHeld READ chargeLimitHeld NOTIFY chargeLimitChanged)

public:
    explicit Battery(QObject *parent = nullptr);

    bool ready() const;
    bool available() const;
    bool onBattery() const;

    int chargeState() const;
    int chargePercent() const;
    int lastChargedPercent() const;
    int capacity() const;
    QString statusString() const;
    QString lastChargedTime() const;

    bool showPercent();
    Q_INVOKABLE void setPercentEnabled(bool value);

    // UPower's charge limit: only on or off, between thresholds the system sets.
    bool chargeLimitSupported() const;
    bool chargeLimitEnabled() const;
    int chargeLimitStart() const;
    int chargeLimitEnd() const;
    // False when UPower has the limit on but the kernel does not: some battery
    // controllers take the thresholds and reset them a few seconds later.
    bool chargeLimitHeld() const;
    Q_INVOKABLE void setChargeLimitEnabled(bool enabled);

    QString udi() const;

signals:
    void readyChanged();
    void validChanged();
    void chargeStateChanged();
    void chargePercentChanged();
    void capacityChanged();
    void remainingTimeChanged();
    void onBatteryChanged();
    void lastChargedPercentChanged();
    void udiChanged();
    void chargeLimitChanged();

private slots:
    void onDevicePropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);

private:
    void watchDevice();

    BatteryStatus *m_status;
    QString m_udi;

    bool m_chargeLimitSupported = false;
    bool m_chargeLimitEnabled = false;
    int m_chargeLimitStart = 0;
    int m_chargeLimitEnd = 0;
    // The battery's directory in /sys/class/power_supply, and the end
    // threshold the kernel reports there; -1 when it cannot be read.
    QString m_batteryPath;
    int m_kernelChargeLimit = -1;

    void loadChargeLimit();
    void updateChargeLimit(const QVariantMap &props);
    void readKernelChargeLimit();
};

#endif // BATTERY_H
