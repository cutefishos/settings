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

#ifndef BATTERY_H
#define BATTERY_H

#include <QObject>
#include <QDBusInterface>

class Battery : public QObject
{
    Q_OBJECT
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
    Q_PROPERTY(int chargeLimit READ chargeLimit NOTIFY chargeLimitChanged)
    Q_PROPERTY(int chargeResume READ chargeResume NOTIFY chargeLimitChanged)

public:
    explicit Battery(QObject *parent = nullptr);

    bool available() const;
    bool onBattery() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantList getHistory(const int timespan, const int resolution);

    int chargeState() const;
    int chargePercent() const;
    int lastChargedPercent() const;
    int capacity() const;
    QString statusString() const;
    QString lastChargedTime() const;

    bool showPercent();
    Q_INVOKABLE void setPercentEnabled(bool value);

    // The kernel's charge thresholds: charging stops at chargeLimit (100 is
    // none) and resumes below chargeResume (0 when the driver decides).
    bool chargeLimitSupported() const;
    int chargeLimit() const;
    int chargeResume() const;
    Q_INVOKABLE void setChargeLimit(int percent);

    QString udi() const;

signals:
    void validChanged();
    void chargeStateChanged(int);
    void chargePercentChanged(int);
    void capacityChanged(int);
    void remainingTimeChanged(qlonglong time);
    void onBatteryChanged();
    void lastChargedPercentChanged();
    void udiChanged();
    void chargeLimitChanged();

private slots:
    void onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);
    void onDevicePropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);

private:
    QDBusInterface m_upowerInterface;
    QDBusInterface m_interface;
    bool m_available;
    bool m_onBattery;
    QString m_udi;

    // The battery's directory in /sys/class/power_supply.
    QString m_batteryPath;
    bool m_upowerLimitEnabled = false;
    bool m_chargeLimitSupported = false;
    int m_chargeLimit = 100;
    int m_chargeResume = 0;
    bool m_chargeLimitBusy = false;
    int m_requestedChargeLimit = -1;

    void loadBatteryDevice();
    void readChargeLimit(bool notify);
    void applyChargeLimit(int percent);
    void runChargeLimitHelper(int percent);
    void finishChargeLimit();
};

#endif // BATTERY_H
