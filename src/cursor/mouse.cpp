/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#include "mouse.h"
#include <QDBusPendingCall>

Mouse::Mouse(QObject *parent)
    : QObject(parent)
    , m_interface("com.cutefish.Settings",
                  "/Mouse",
                  "com.cutefish.Mouse",
                  QDBusConnection::sessionBus())
{
    if (m_interface.isValid()) {
        connect(&m_interface, SIGNAL(leftHandedChanged()), this, SIGNAL(leftHandedChanged()));
        connect(&m_interface, SIGNAL(naturalScrollChanged()), this, SIGNAL(naturalScrollChanged()));
        connect(&m_interface, SIGNAL(pointerAccelerationChanged()), this, SIGNAL(pointerAccelerationChanged()));
    }
}

Mouse::~Mouse()
{
}

bool Mouse::leftHanded() const
{
    return m_interface.property("leftHanded").toBool();
}

void Mouse::setLeftHanded(bool enabled)
{
    m_interface.asyncCall("setLeftHanded", enabled);
}

bool Mouse::acceleration() const
{
    return m_interface.property("acceleration").toBool();
}

void Mouse::setAcceleration(bool enabled)
{
    m_interface.asyncCall("setPointerAccelerationProfileFlat", enabled);
}

bool Mouse::naturalScroll() const
{
    return m_interface.property("naturalScroll").toBool();
}

void Mouse::setNaturalScroll(bool enabled)
{
    m_interface.asyncCall("setNaturalScroll", enabled);
}

qreal Mouse::pointerAcceleration() const
{
    return m_interface.property("pointerAcceleration").toReal();
}

void Mouse::setPointerAcceleration(qreal value)
{
    m_interface.asyncCall("setPointerAcceleration", value);
}
