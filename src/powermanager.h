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

#ifndef POWERMANAGER_H
#define POWERMANAGER_H

#include <QObject>
#include <QStringList>
#include <QVariantMap>

class PowerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString profile READ profile WRITE setProfile NOTIFY profileChanged)
    Q_PROPERTY(QStringList profiles READ profiles NOTIFY profilesChanged)
    Q_PROPERTY(QString performanceDegraded READ performanceDegraded NOTIFY performanceDegradedChanged)
    Q_PROPERTY(int batteryScreenOff READ batteryScreenOff WRITE setBatteryScreenOff NOTIFY batteryScreenOffChanged)
    Q_PROPERTY(int acScreenOff READ acScreenOff WRITE setACScreenOff NOTIFY acScreenOffChanged)
    Q_PROPERTY(bool acPreventSleep READ acPreventSleep WRITE setACPreventSleep NOTIFY acPreventSleepChanged)

public:
    explicit PowerManager(QObject *parent = nullptr);

    QString profile() const;
    void setProfile(const QString &profile);

    QStringList profiles() const;
    QString performanceDegraded() const;

    int batteryScreenOff() const;
    void setBatteryScreenOff(int timeout);

    int acScreenOff() const;
    void setACScreenOff(int timeout);

    bool acPreventSleep() const;
    void setACPreventSleep(bool prevent);

signals:
    void profileChanged();
    void profilesChanged();
    void performanceDegradedChanged();
    void batteryScreenOffChanged();
    void acScreenOffChanged();
    void acPreventSleepChanged();

private slots:
    void onProfilePropertiesChanged(const QString &interface, const QVariantMap &changed,
                                    const QStringList &invalidated);

private:
    void loadProfileProperties();
    void updateProfileProperties(const QVariantMap &properties);

    QString m_profile;
    QStringList m_profiles;
    QString m_performanceDegraded;
    int m_batteryScreenOff;
    int m_acScreenOff;
    bool m_acPreventSleep;
};

#endif // POWERMANAGER_H
