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

DockSettings::DockSettings(QObject *parent)
    : QObject(parent)
    , m_dockSettings(new QSettings(QSettings::UserScope, "cutefishos", "dock"))
    , m_dockIconSize(0)
    , m_dockDirection(0)
    , m_dockVisibility(0)
{
    m_dockIconSize = m_dockSettings->value("IconSize").toInt();
    m_dockDirection = m_dockSettings->value("Direction").toInt();
    m_dockVisibility = m_dockSettings->value("Visibility").toInt();
    m_dockRoundedWindow = m_dockSettings->value("RoundedWindow").toBool();
    // Dock only supports the centered style. Migrate old Full style settings.
    m_dockSettings->setValue("Style", 0);
}

int DockSettings::dockIconSize() const
{
    return m_dockIconSize;
}

void DockSettings::setDockIconSize(int dockIconSize)
{
    if (m_dockIconSize != dockIconSize) {
        QDBusInterface iface("com.cutefish.Dock",
                             "/Dock",
                             "com.cutefish.Dock",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("setIconSize", dockIconSize);
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
        QDBusInterface iface("com.cutefish.Dock",
                             "/Dock",
                             "com.cutefish.Dock",
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

        QDBusInterface iface("com.cutefish.Dock",
                             "/Dock",
                             "com.cutefish.Dock",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("setVisibility", visibility);
        }

        emit dockVisibilityChanged();
    }
}

int DockSettings::dockRoundedWindow() const
{
    return m_dockRoundedWindow;
}

void DockSettings::setDockRoundedWindow(bool enable)
{
    if (m_dockRoundedWindow == enable)
        return;

    m_dockRoundedWindow = enable;
    m_dockSettings->setValue("RoundedWindow", m_dockRoundedWindow);
}
