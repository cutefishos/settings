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

class DockSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int dockIconSize READ dockIconSize WRITE setDockIconSize NOTIFY dockIconSizeChanged)
    Q_PROPERTY(
            int dockDirection READ dockDirection WRITE setDockDirection NOTIFY dockDirectionChanged)
    Q_PROPERTY(int dockVisibility READ dockVisibility WRITE setDockVisibility NOTIFY
                       dockVisibilityChanged)
    Q_PROPERTY(qreal dockGlassTint READ dockGlassTint WRITE setDockGlassTint NOTIFY
                       dockGlassTintChanged)

public:
    explicit DockSettings(QObject *parent = nullptr);

    int dockIconSize() const;
    Q_INVOKABLE void setDockIconSize(int dockIconSize);

    int dockDirection() const;
    Q_INVOKABLE void setDockDirection(int dockDirection);

    int dockVisibility() const;
    Q_INVOKABLE void setDockVisibility(int visibility);

    qreal dockGlassTint() const;
    Q_INVOKABLE void setDockGlassTint(qreal tint);

signals:
    void dockIconSizeChanged();
    void dockDirectionChanged();
    void dockVisibilityChanged();
    void dockGlassTintChanged();

private:
    int m_dockIconSize;
    int m_dockDirection;
    int m_dockVisibility;
    qreal m_dockGlassTint;
};

#endif // DOCKSETTINGS_H
