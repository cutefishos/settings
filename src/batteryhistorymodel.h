// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BATTERYHISTORYMODEL_H
#define BATTERYHISTORYMODEL_H

#include <QObject>
#include <QQmlParserStatus>
#include <QPointF>

struct HistoryReply {
public:
    uint time = 0;
    double value = 0.0;
    uint charging = 0;
};

Q_DECLARE_METATYPE(HistoryReply)

class BatteryHistoryModel : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_ENUMS(HistoryType)

    Q_PROPERTY(QString device MEMBER m_device WRITE setDevice NOTIFY deviceChanged)
    Q_PROPERTY(uint duration MEMBER m_duration WRITE setDuration NOTIFY durationChanged)
    Q_PROPERTY(HistoryType type MEMBER m_type WRITE setType NOTIFY typeChanged)

    Q_PROPERTY(QVariantList points READ asPoints NOTIFY dataChanged)
    // UPower's device state when each point was recorded, e.g. 1 for charging.
    Q_PROPERTY(QVariantList states READ states NOTIFY dataChanged)
    Q_PROPERTY(int count READ count NOTIFY dataChanged)
    Q_PROPERTY(int lastDataPointTime READ lastDataPointTime NOTIFY dataChanged)
    Q_PROPERTY(int largestValue READ largestValue NOTIFY dataChanged)

public:
    enum HistoryType {
        RateType,
        ChargeType
    };

    enum HistoryRoles {
        TimeRole = Qt::UserRole + 1,
        ValueRole,
        ChargingRole
    };

    explicit BatteryHistoryModel(QObject *parent = nullptr);

    void setDevice(const QString &device);
    void setDuration(uint duration);
    void setType(HistoryType type);

    void load();

    void classBegin() override;
    void componentComplete() override;

    QVariantList asPoints() const;
    QVariantList states() const;
    int count() const;

    int lastDataPointTime() const;
    int largestValue() const;

Q_SIGNALS:
    void deviceChanged();
    void typeChanged();
    void durationChanged();

    void dataChanged();

public Q_SLOTS:
    void refresh();

private:
    QString m_device;
    HistoryType m_type;
    uint m_duration; //in seconds

    QList<HistoryReply> m_values;
    bool m_isComplete = false;
};

#endif
