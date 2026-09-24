/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
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

#include "touchpad.h"
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDebug>

namespace {
constexpr auto service = "com.cutefish.Touchpad1";
constexpr auto path = "/com/cutefish/Touchpad1";
} // namespace

Touchpad::Touchpad(QObject *parent)
    : QObject(parent)
{
    auto *watcher = new QDBusServiceWatcher(QLatin1String(service), QDBusConnection::sessionBus(),
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this,
            &Touchpad::refreshInterface);
    refreshInterface();
}

void Touchpad::refreshInterface()
{
    delete m_interface;
    m_interface = new QDBusInterface(QLatin1String(service), QLatin1String(path),
                                     QLatin1String(service), QDBusConnection::sessionBus(), this);
    if (m_interface->isValid()) {
        connect(m_interface, SIGNAL(availableChanged()), this, SIGNAL(availableChanged()));
        connect(m_interface, SIGNAL(enabledChanged()), this, SIGNAL(enabledChanged()));
        connect(m_interface, SIGNAL(disableWithMouseChanged()), this,
                SIGNAL(disableWithMouseChanged()));
        connect(m_interface, SIGNAL(disableWhileTypingChanged()), this,
                SIGNAL(disableWhileTypingChanged()));
        connect(m_interface, SIGNAL(tapToClickChanged()), this, SIGNAL(tapToClickChanged()));
        connect(m_interface, SIGNAL(tapDraggingChanged()), this, SIGNAL(tapDraggingChanged()));
        connect(m_interface, SIGNAL(naturalScrollChanged()), this, SIGNAL(naturalScrollChanged()));
        connect(m_interface, SIGNAL(pointerAccelerationChanged()), this,
                SIGNAL(pointerAccelerationChanged()));
    }
    notifyChanged();
}

void Touchpad::notifyChanged()
{
    emit availableChanged();
    emit enabledChanged();
    emit disableWithMouseChanged();
    emit disableWhileTypingChanged();
    emit tapToClickChanged();
    emit tapDraggingChanged();
    emit naturalScrollChanged();
    emit pointerAccelerationChanged();
}

void Touchpad::call(const QString &method, const QVariant &value)
{
    if (!m_interface->isValid())
        return;
    auto *watcher = new QDBusPendingCallWatcher(m_interface->asyncCall(method, value), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, method](QDBusPendingCallWatcher *reply) {
                const QDBusPendingReply<> result = *reply;
                if (result.isError()) {
                    qWarning() << "Cannot update touchpad setting" << method
                               << result.error().message();
                    notifyChanged();
                }
                reply->deleteLater();
            });
}

// Without the service every switch shows the compositor's default.
bool Touchpad::flag(const char *name, bool fallback) const
{
    return m_interface->isValid() ? m_interface->property(name).toBool() : fallback;
}

bool Touchpad::available() const
{
    return flag("available", false);
}

bool Touchpad::enabled() const
{
    return flag("enabled", true);
}

void Touchpad::setEnabled(bool enabled)
{
    call(QStringLiteral("setEnabled"), enabled);
}

bool Touchpad::disableWithMouse() const
{
    return flag("disableWithMouse", false);
}

void Touchpad::setDisableWithMouse(bool enabled)
{
    call(QStringLiteral("setDisableWithMouse"), enabled);
}

bool Touchpad::disableWhileTyping() const
{
    return flag("disableWhileTyping", true);
}

void Touchpad::setDisableWhileTyping(bool enabled)
{
    call(QStringLiteral("setDisableWhileTyping"), enabled);
}

bool Touchpad::tapToClick() const
{
    return flag("tapToClick", true);
}

void Touchpad::setTapToClick(bool enabled)
{
    call(QStringLiteral("setTapToClick"), enabled);
}

bool Touchpad::tapDragging() const
{
    return flag("tapDragging", false);
}

void Touchpad::setTapDragging(bool enabled)
{
    call(QStringLiteral("setTapDragging"), enabled);
}

bool Touchpad::naturalScroll() const
{
    return flag("naturalScroll", false);
}

void Touchpad::setNaturalScroll(bool enabled)
{
    call(QStringLiteral("setNaturalScroll"), enabled);
}

qreal Touchpad::pointerAcceleration() const
{
    return m_interface->isValid() ? m_interface->property("pointerAcceleration").toReal() : 0;
}

void Touchpad::setPointerAcceleration(qreal value)
{
    call(QStringLiteral("setPointerAcceleration"), value);
}
