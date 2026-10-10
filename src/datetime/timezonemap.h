// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEZONEMAP_H
#define TIMEZONEMAP_H

#include <QObject>
#include <QPointF>
#include <QVariantList>
#include <QVariantMap>

class TimeZoneItem {
public:
    QString country;
    QString timeZone;
    double latitude;
    double longitude;
};

class TimeZoneMap : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList availableList READ availableList NOTIFY availableListChanged)
    Q_PROPERTY(QString currentTimeZone READ currentTimeZone NOTIFY currentTimeZoneChanged)
    Q_PROPERTY(QString currentTimeZoneId READ currentTimeZoneId NOTIFY currentTimeZoneChanged)

public:
    explicit TimeZoneMap(QObject *parent = nullptr);
    ~TimeZoneMap() override;

    Q_INVOKABLE void clicked(int x, int y, int width, int height);
    Q_INVOKABLE void setTimeZone(QString value);

    // Makes `timeZone` the only candidate, as a click on its spot would.
    Q_INVOKABLE void select(const QString &timeZone);

    // Where `timeZone` sits on a map of the given size; (-1, -1) if unknown.
    Q_INVOKABLE QPointF position(const QString &timeZone, int width, int height) const;

    // The zone nearest to a point on a map of the given size, without
    // selecting it; empty if there are no zones.
    Q_INVOKABLE QString nearest(int x, int y, int width, int height) const;

    // Every zone as [{id, x, y, offsetSeconds}], with x and y as fractions
    // of the map's size and the offset in force now.
    Q_INVOKABLE QVariantList points() const;

    // {id, name, region, offsetSeconds, time, day, difference} for one zone:
    // `time` is the wall clock there now, `day` says "Tomorrow" or "Yesterday"
    // when its date differs from here, and `difference` puts the gap to the
    // zone in use in words. Empty if unknown.
    Q_INVOKABLE QVariantMap info(const QString &timeZone) const;

    // Zones whose id, city or country matches `text`, best matches first,
    // each as info() gives it. Empty text gives nothing.
    Q_INVOKABLE QVariantList search(const QString &text, int limit = 12) const;

    QString currentTimeZone() const;
    QString currentTimeZoneId() const { return m_currentTimeZone; }
    QStringList availableList();

    Q_INVOKABLE QString localeTimeZoneName(const QString &timeZone) const;

signals:
    void availableListChanged();
    void currentTimeZoneChanged();

private slots:
    void onPropertiesChanged(const QString &interface, const QVariantMap &changed,
                             const QStringList &invalidated);

private:
    void initDatas();
    void setCurrentTimeZone(const QString &value);
    int nearestIndex(int x, int y, int width, int height) const;

private:
    QList<TimeZoneItem *> m_list;
    QString m_currentTimeZone;
    QStringList m_currentList;
};

#endif // TIMEZONEMAP_H
