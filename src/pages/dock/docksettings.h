// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "base/dbus.h"

namespace settings {

// The dock's settings: read from the shell's file, changed through the dock,
// which owns that file.
class DockSettings
{
public:
    DockSettings();

    int iconSize() const { return m_iconSize; }
    void setIconSize(int size);

    // 0 left, 1 bottom, 2 right.
    int direction() const { return m_direction; }
    void setDirection(int direction);

    // 0 always shown, 1 always hidden, 2 smart hide.
    int visibility() const { return m_visibility; }
    void setVisibility(int visibility);

    double glassTint() const { return m_glassTint; }
    void setGlassTint(double tint);

    bool minimizedArea() const { return m_minimizedArea; }
    void setMinimizedArea(bool enabled);

private:
    cutefish::base::dbus::Proxy m_dock;
    int m_iconSize = 0;
    int m_direction = 0;
    int m_visibility = 0;
    double m_glassTint = 0.5;
    bool m_minimizedArea = true;
};

} // namespace settings
