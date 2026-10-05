// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mouse.h"
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDebug>

static constexpr auto service = "com.cutefish.Mouse1";
static constexpr auto path = "/com/cutefish/Mouse1";

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
