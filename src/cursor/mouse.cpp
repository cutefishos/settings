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
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDebug>

namespace {
constexpr auto service = "com.cutefish.Mouse1";
constexpr auto path = "/com/cutefish/Mouse1";
} // namespace

Mouse::Mouse(QObject *parent)
    : QObject(parent)
{
    auto *watcher = new QDBusServiceWatcher(QLatin1String(service), QDBusConnection::sessionBus(),
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &Mouse::refreshInterface);
    refreshInterface();
}

Mouse::~Mouse()
{
}

void Mouse::refreshInterface()
{
    delete m_interface;
    m_interface = new QDBusInterface(QLatin1String(service), QLatin1String(path),
                                     QLatin1String(service), QDBusConnection::sessionBus(), this);
    if (m_interface->isValid()) {
        connect(m_interface, SIGNAL(availableChanged()), this, SIGNAL(availableChanged()));
        connect(m_interface, SIGNAL(leftHandedChanged()), this, SIGNAL(leftHandedChanged()));
        connect(m_interface, SIGNAL(accelerationChanged()), this, SIGNAL(accelerationChanged()));
        connect(m_interface, SIGNAL(naturalScrollChanged()), this, SIGNAL(naturalScrollChanged()));
        connect(m_interface, SIGNAL(pointerAccelerationChanged()), this,
                SIGNAL(pointerAccelerationChanged()));
    }
    notifyChanged();
}

void Mouse::notifyChanged()
{
    emit availableChanged();
    emit leftHandedChanged();
    emit accelerationChanged();
    emit naturalScrollChanged();
    emit pointerAccelerationChanged();
}

void Mouse::call(const QString &method, const QVariant &value)
{
    if (!m_interface->isValid())
        return;
    auto *watcher = new QDBusPendingCallWatcher(m_interface->asyncCall(method, value), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, method](QDBusPendingCallWatcher *reply) {
                const QDBusPendingReply<> result = *reply;
                if (result.isError()) {
                    qWarning() << "Cannot update mouse setting" << method
                               << result.error().message();
                    notifyChanged();
                }
                reply->deleteLater();
            });
}

bool Mouse::available() const
{
    return m_interface->isValid() && m_interface->property("available").toBool();
}

bool Mouse::leftHanded() const
{
    return m_interface->property("leftHanded").toBool();
}

void Mouse::setLeftHanded(bool enabled)
{
    call(QStringLiteral("setLeftHanded"), enabled);
}

bool Mouse::acceleration() const
{
    return m_interface->isValid() ? m_interface->property("acceleration").toBool() : true;
}

void Mouse::setAcceleration(bool enabled)
{
    call(QStringLiteral("setAcceleration"), enabled);
}

bool Mouse::naturalScroll() const
{
    return m_interface->property("naturalScroll").toBool();
}

void Mouse::setNaturalScroll(bool enabled)
{
    call(QStringLiteral("setNaturalScroll"), enabled);
}

qreal Mouse::pointerAcceleration() const
{
    return m_interface->property("pointerAcceleration").toReal();
}

void Mouse::setPointerAcceleration(qreal value)
{
    call(QStringLiteral("setPointerAcceleration"), value);
}
