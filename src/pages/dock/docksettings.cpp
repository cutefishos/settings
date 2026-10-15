// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/dock/docksettings.h"

#include <algorithm>
#include <cstdint>

#include "base/paths.h"
#include "base/settings.h"

using namespace cutefish::base;

namespace settings {

DockSettings::DockSettings()
    : m_dock(dbus::Connection::session(), "com.cutefish.Dock", "/Dock", "com.cutefish.Dock")
{
    const Settings shell(paths::cutefishConfig("shell"));
    m_iconSize = int(shell.intValue("Dock/IconSize"));
    m_direction = int(shell.intValue("Dock/Direction"));
    m_visibility = int(shell.intValue("Dock/Visibility"));
    m_glassTint = std::clamp(shell.doubleValue("Dock/GlassTint", 0.5), 0.0, 1.0);
    m_minimizedArea = shell.boolValue("Dock/MinimizedArea", true);
}

// Called on every slider move; none of these waits for the dock.
void DockSettings::setIconSize(int size)
{
    if (size == m_iconSize)
        return;
    m_dock.send("setIconSize", int32_t(size));
    m_iconSize = size;
}

void DockSettings::setDirection(int direction)
{
    if (direction == m_direction)
        return;
    m_dock.send("setDirection", int32_t(direction));
    m_direction = direction;
}

void DockSettings::setVisibility(int visibility)
{
    if (visibility == m_visibility)
        return;
    m_dock.send("setVisibility", int32_t(visibility));
    m_visibility = visibility;
}

void DockSettings::setGlassTint(double tint)
{
    tint = std::clamp(tint, 0.0, 1.0);
    if (tint == m_glassTint)
        return;
    m_dock.send("setGlassTint", tint);
    m_glassTint = tint;
}

void DockSettings::setMinimizedArea(bool enabled)
{
    if (enabled == m_minimizedArea)
        return;
    m_dock.send("setMinimizedArea", enabled);
    m_minimizedArea = enabled;
}

} // namespace settings
