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

#include "appearance.h"

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDBusPendingCall>

#include <QStandardPaths>
#include <QDebug>

Appearance::Appearance(QObject *parent)
    : QObject(parent)
    , m_interface("org.cutefish.Settings",
                  "/Theme",
                  "org.cutefish.Theme",
                  QDBusConnection::sessionBus())
    , m_dockSettings(new QSettings(QSettings::UserScope, "cutefishos", "dock"))
    , m_kwinSettings(new QSettings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kwinrc",
                                   QSettings::IniFormat))
    , m_dockIconSize(0)
    , m_dockDirection(0)
    , m_dockVisibility(0)
    , m_fontPointSize(11)
    , m_systemEffects(false)
{
    m_kwinSettings->beginGroup("Compositing");

    m_dockIconSize = m_dockSettings->value("IconSize").toInt();
    m_dockDirection = m_dockSettings->value("Direction").toInt();
    m_dockVisibility = m_dockSettings->value("Visibility").toInt();
    m_dockRoundedWindow = m_dockSettings->value("RoundedWindow").toBool();
    m_systemEffects = !m_kwinSettings->value("OpenGLIsUnsafe", false).toBool();

    // Init
    if (m_interface.isValid()) {
        m_fontPointSize = m_interface.property("systemFontPointSize").toInt();

        connect(&m_interface, SIGNAL(darkModeDimsWallpaerChanged()), this, SIGNAL(dimsWallpaperChanged()));
    }
}

void Appearance::switchDarkMode(bool darkMode)
{
    if (m_interface.isValid()) {
        m_interface.call("setDarkMode", darkMode);
    }
}

bool Appearance::dimsWallpaper() const
{
    return m_interface.property("darkModeDimsWallpaer").toBool();
}

void Appearance::setDimsWallpaper(bool value)
{
    m_interface.call("setDarkModeDimsWallpaer", value);
}

int Appearance::dockIconSize() const
{
    return m_dockIconSize;
}

void Appearance::setDockIconSize(int dockIconSize)
{
    if (m_dockIconSize != dockIconSize) {
        QDBusInterface iface("org.cutefish.Dock",
                             "/Dock",
                             "org.cutefish.Dock",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("setIconSize", dockIconSize);
        }

        m_dockIconSize = dockIconSize;
        emit dockIconSizeChanged();
    }
}

int Appearance::dockDirection() const
{
    return m_dockDirection;
}

void Appearance::setDockDirection(int dockDirection)
{
    if (m_dockDirection != dockDirection) {
        QDBusInterface iface("org.cutefish.Dock",
                             "/Dock",
                             "org.cutefish.Dock",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("setDirection", dockDirection);
        }

        m_dockDirection = dockDirection;
        emit dockDirectionChanged();
    }
}

int Appearance::dockVisibility() const
{
    return m_dockVisibility;
}

void Appearance::setDockVisibility(int visibility)
{
    if (m_dockVisibility != visibility) {
        m_dockVisibility = visibility;

        QDBusInterface iface("org.cutefish.Dock",
                             "/Dock",
                             "org.cutefish.Dock",
                             QDBusConnection::sessionBus());
        if (iface.isValid()) {
            iface.call("setVisibility", visibility);
        }

        emit dockVisibilityChanged();
    }
}

int Appearance::dockRoundedWindow() const
{
    return m_dockRoundedWindow;
}

void Appearance::setDockRoundedWindow(bool enable)
{
    if (m_dockRoundedWindow == enable)
        return;

    m_dockRoundedWindow = enable;
    m_dockSettings->setValue("RoundedWindow", m_dockRoundedWindow);
}

void Appearance::setGenericFontFamily(const QString &name)
{
    if (name.isEmpty())
        return;

    QDBusInterface iface("org.cutefish.Settings",
                         "/Theme",
                         "org.cutefish.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setSystemFont", name);
    }
}

void Appearance::setFixedFontFamily(const QString &name)
{
    if (name.isEmpty())
        return;

    QDBusInterface iface("org.cutefish.Settings",
                         "/Theme",
                         "org.cutefish.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setSystemFixedFont", name);
    }
}

int Appearance::fontPointSize() const
{
    return m_fontPointSize;
}

void Appearance::setFontPointSize(int fontPointSize)
{
    m_fontPointSize = fontPointSize;

    QDBusInterface iface("org.cutefish.Settings",
                         "/Theme",
                         "org.cutefish.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setSystemFontPointSize", m_fontPointSize * 1.0);
    }
}

void Appearance::setAccentColor(int accentColor)
{
    QDBusInterface iface("org.cutefish.Settings",
                         "/Theme",
                         "org.cutefish.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setAccentColor", accentColor);
    }
}

double Appearance::devicePixelRatio() const
{
    return m_interface.property("devicePixelRatio").toDouble();
}

void Appearance::setDevicePixelRatio(double value)
{
    QDBusInterface iface("org.cutefish.Settings",
                         "/Theme",
                         "org.cutefish.Theme",
                         QDBusConnection::sessionBus(), this);
    if (iface.isValid()) {
        iface.call("setDevicePixelRatio", value);
    }
}

bool Appearance::systemEffects() const
{
    return m_systemEffects;
}

void Appearance::setSystemEffects(bool systemEffects)
{
    if (m_systemEffects != systemEffects) {
        m_systemEffects = systemEffects;
        m_kwinSettings->setValue("OpenGLIsUnsafe", !systemEffects);
        m_kwinSettings->sync();
        QDBusInterface("org.kde.KWin", "/KWin").call("reconfigure");
        emit systemEffectsChanged();
    }
}
