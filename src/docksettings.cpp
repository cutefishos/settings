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

#include "docksettings.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QSettings>

#include <preferences.h>

DockSettings::DockSettings(QObject *parent)
    : QObject(parent)
    , m_dockIconSize(0)
    , m_dockDirection(0)
    , m_dockVisibility(0)
    , m_dockGlassTint(0.5)
{
    QSettings settings(Preferences::file(QStringLiteral("shell")), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Dock"));
    m_dockIconSize = settings.value("IconSize").toInt();
    m_dockDirection = settings.value("Direction").toInt();
    m_dockVisibility = settings.value("Visibility").toInt();
    m_dockGlassTint = qBound(0.0, settings.value("GlassTint", 0.5).toDouble(), 1.0);
}

int DockSettings::dockIconSize() const
{
    return m_dockIconSize;
}

void DockSettings::setDockIconSize(int dockIconSize)
{
    if (m_dockIconSize != dockIconSize) {
        // Called on every slider move; do not block on the dock.
        QDBusInterface iface("com.cutefish.Dock", "/Dock", "com.cutefish.Dock",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.asyncCall("setIconSize", dockIconSize);
        }

        m_dockIconSize = dockIconSize;
        emit dockIconSizeChanged();
    }
}

int DockSettings::dockDirection() const
{
    return m_dockDirection;
}

void DockSettings::setDockDirection(int dockDirection)
{
    if (m_dockDirection != dockDirection) {
        QDBusInterface iface("com.cutefish.Dock", "/Dock", "com.cutefish.Dock",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("setDirection", dockDirection);
        }

        m_dockDirection = dockDirection;
        emit dockDirectionChanged();
    }
}

int DockSettings::dockVisibility() const
{
    return m_dockVisibility;
}

void DockSettings::setDockVisibility(int visibility)
{
    if (m_dockVisibility != visibility) {
        m_dockVisibility = visibility;

        QDBusInterface iface("com.cutefish.Dock", "/Dock", "com.cutefish.Dock",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("setVisibility", visibility);
        }

        emit dockVisibilityChanged();
    }
}

qreal DockSettings::dockGlassTint() const
{
    return m_dockGlassTint;
}

void DockSettings::setDockGlassTint(qreal tint)
{
    tint = qBound(0.0, tint, 1.0);
    if (qFuzzyCompare(m_dockGlassTint, tint))
        return;

    // Called on every slider move; do not block on the dock.
    QDBusInterface iface("com.cutefish.Dock", "/Dock", "com.cutefish.Dock",
                         QDBusConnection::sessionBus());
    if (iface.isValid())
        iface.asyncCall("setGlassTint", tint);

    m_dockGlassTint = tint;
    emit dockGlassTintChanged();
}
