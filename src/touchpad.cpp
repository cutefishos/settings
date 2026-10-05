// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "touchpad.h"
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDebug>

static constexpr auto service = "com.cutefish.Touchpad1";
static constexpr auto path = "/com/cutefish/Touchpad1";

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
        connect(m_interface, SIGNAL(scrollSpeedChanged()), this, SIGNAL(scrollSpeedChanged()));
        connect(m_interface, SIGNAL(secondaryClickConfigurableChanged()), this,
                SIGNAL(secondaryClickConfigurableChanged()));
        connect(m_interface, SIGNAL(twoFingerSecondaryClickChanged()), this,
                SIGNAL(twoFingerSecondaryClickChanged()));
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
    emit scrollSpeedChanged();
    emit secondaryClickConfigurableChanged();
    emit twoFingerSecondaryClickChanged();
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
    return flag("tapToClick", false);
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
    return flag("naturalScroll", true);
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

qreal Touchpad::scrollSpeed() const
{
    // The compositor's default: content under the fingers at 150%.
    return m_interface->isValid() ? m_interface->property("scrollSpeed").toReal() : 0.5;
}

void Touchpad::setScrollSpeed(qreal value)
{
    call(QStringLiteral("setScrollSpeed"), value);
}

bool Touchpad::secondaryClickConfigurable() const
{
    return flag("secondaryClickConfigurable", false);
}

bool Touchpad::twoFingerSecondaryClick() const
{
    return flag("twoFingerSecondaryClick", true);
}

void Touchpad::setTwoFingerSecondaryClick(bool enabled)
{
    call(QStringLiteral("setTwoFingerSecondaryClick"), enabled);
}
