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

#ifndef DOCKSETTINGS_H
#define DOCKSETTINGS_H

#include <QObject>
#include <QSettings>

class DockSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int dockIconSize READ dockIconSize WRITE setDockIconSize NOTIFY dockIconSizeChanged)
    Q_PROPERTY(int dockDirection READ dockDirection WRITE setDockDirection NOTIFY dockDirectionChanged)
    Q_PROPERTY(int dockVisibility READ dockVisibility WRITE setDockVisibility NOTIFY dockVisibilityChanged)
    Q_PROPERTY(bool dockRoundedWindow READ dockRoundedWindow WRITE setDockRoundedWindow NOTIFY dockRoundedWindowChanged)

public:
    explicit DockSettings(QObject *parent = nullptr);

    int dockIconSize() const;
    Q_INVOKABLE void setDockIconSize(int dockIconSize);

    int dockDirection() const;
    Q_INVOKABLE void setDockDirection(int dockDirection);

    int dockVisibility() const;
    Q_INVOKABLE void setDockVisibility(int visibility);

    int dockRoundedWindow() const;
    Q_INVOKABLE void setDockRoundedWindow(bool enable);


signals:
    void dockIconSizeChanged();
    void dockDirectionChanged();
    void dockVisibilityChanged();
    void dockRoundedWindowChanged();

private:
    QSettings *m_dockSettings;
    bool m_dockRoundedWindow;

    int m_dockIconSize;
    int m_dockDirection;
    int m_dockVisibility;
};

#endif // DOCKSETTINGS_H
