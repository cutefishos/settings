// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

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
    , m_minimizedArea(true)
{
    QSettings settings(Preferences::file(QStringLiteral("shell")), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Dock"));
    m_dockIconSize = settings.value("IconSize").toInt();
    m_dockDirection = settings.value("Direction").toInt();
    m_dockVisibility = settings.value("Visibility").toInt();
    m_dockGlassTint = qBound(0.0, settings.value("GlassTint", 0.5).toDouble(), 1.0);
    m_minimizedArea = settings.value("MinimizedArea", true).toBool();
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

bool DockSettings::minimizedArea() const
{
    return m_minimizedArea;
}

void DockSettings::setMinimizedArea(bool enabled)
{
    if (m_minimizedArea == enabled)
        return;

    QDBusInterface iface("com.cutefish.Dock", "/Dock", "com.cutefish.Dock",
                         QDBusConnection::sessionBus());
    if (iface.isValid())
        iface.asyncCall("setMinimizedArea", enabled);

    m_minimizedArea = enabled;
    emit minimizedAreaChanged();
}
