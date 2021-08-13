/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#ifndef TIMEZONEMAP_H
#define TIMEZONEMAP_H

#include <QObject>

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

public:
    explicit TimeZoneMap(QObject *parent = nullptr);

    Q_INVOKABLE void clicked(int x, int y, int width, int height);
    Q_INVOKABLE void setTimeZone(QString value);

    QString currentTimeZone() const;
    QStringList availableList();

    Q_INVOKABLE QString localeTimeZoneName(const QString &timeZone) const;

signals:
    void availableListChanged();
    void currentTimeZoneChanged();

private:
    void initDatas();

private:
    QList<TimeZoneItem *> m_list;
    QString m_currentTimeZone;
    QStringList m_currentList;
};

#endif // TIMEZONEMAP_H
