// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/mouse/mouse.h"

namespace settings {

Mouse::Mouse()
    : m_service("org.cutefish.Mouse1")
{
}

bool Mouse::isAvailable() const
{
    return m_service.isAvailable() && m_service.boolValue("available", false);
}

bool Mouse::leftHanded() const
{
    return m_service.boolValue("leftHanded", false);
}

void Mouse::setLeftHanded(bool enabled)
{
    m_service.call("setLeftHanded", enabled);
}

// Without the service the switch shows the compositor's default.
bool Mouse::acceleration() const
{
    return m_service.boolValue("acceleration", true);
}

void Mouse::setAcceleration(bool enabled)
{
    m_service.call("setAcceleration", enabled);
}

bool Mouse::naturalScroll() const
{
    return m_service.boolValue("naturalScroll", false);
}

void Mouse::setNaturalScroll(bool enabled)
{
    m_service.call("setNaturalScroll", enabled);
}

double Mouse::pointerAcceleration() const
{
    return m_service.doubleValue("pointerAcceleration", 0);
}

void Mouse::setPointerAcceleration(double value)
{
    m_service.call("setPointerAcceleration", value);
}

} // namespace settings
