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

#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <QDBusInterface>
#include <QObject>

// Brightness of one display, addressed by its DRM connector name ("card0-eDP-1").
class Brightness : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString display READ display WRITE setDisplay NOTIFY displayChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(int value READ value NOTIFY valueChanged)

public:
    explicit Brightness(QObject *parent = nullptr);

    QString display() const;
    void setDisplay(const QString &display);

    bool available() const;
    int value() const;

    Q_INVOKABLE void setValue(int value);

signals:
    void displayChanged();
    void availableChanged();
    void valueChanged();

private slots:
    void refresh();
    void handleValueChanged(const QString &display, int value);

private:
    QDBusInterface m_iface;
    QString m_display;
    bool m_available = false;
    int m_value = 0;
};

#endif // BRIGHTNESS_H
