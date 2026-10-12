// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "hotcorners.h"

#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDebug>

static constexpr auto service = "com.cutefish.HotCorners1";
static constexpr auto path = "/com/cutefish/HotCorners1";

// The compositor's modifier bits, as in the shortcuts service.
static constexpr int ShiftDown = 1 << 0;
static constexpr int ControlDown = 1 << 1;
static constexpr int AltDown = 1 << 2;
static constexpr int MetaDown = 1 << 3;

HotCorners::HotCorners(QObject *parent)
    : QObject(parent)
{
    auto *watcher = new QDBusServiceWatcher(QLatin1String(service), QDBusConnection::sessionBus(),
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &HotCorners::refreshInterface);
    refreshInterface();

    QDBusInterface shortcuts(QStringLiteral("com.cutefish.Shortcuts1"),
                             QStringLiteral("/com/cutefish/Shortcuts1"),
                             QStringLiteral("com.cutefish.Shortcuts1"), QDBusConnection::sessionBus());
    m_commandKeys = shortcuts.isValid()
            && shortcuts.property("shortcutConvention").toString() == QLatin1String("command");
}

void HotCorners::refreshInterface()
{
    delete m_interface;
    m_interface = new QDBusInterface(QLatin1String(service), QLatin1String(path),
                                     QLatin1String(service), QDBusConnection::sessionBus(), this);
    if (m_interface->isValid())
        connect(m_interface, SIGNAL(changed()), this, SIGNAL(changed()));
    emit changed();
}

bool HotCorners::available() const
{
    return m_interface->isValid();
}

void HotCorners::setCorner(const QString &corner, const QString &action, int modifiers)
{
    if (!m_interface->isValid())
        return;
    auto *watcher = new QDBusPendingCallWatcher(
            m_interface->asyncCall(QStringLiteral("setCorner"), corner, action, modifiers), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this](QDBusPendingCallWatcher *reply) {
        const QDBusPendingReply<> result = *reply;
        if (result.isError()) {
            qWarning() << "Cannot update hot corner" << result.error().message();
            emit changed();
        }
        reply->deleteLater();
    });
}

int HotCorners::heldModifiers() const
{
    if (!m_interface->isValid())
        return 0;
    const QDBusReply<int> reply = m_interface->call(QStringLiteral("heldModifiers"));
    return reply.isValid() ? reply.value() : 0;
}

QString HotCorners::modifierText(int modifiers) const
{
    // Under the Command convention Meta is the Command key, Alt is Option.
    QStringList parts;
    if (modifiers & ControlDown)
        parts << (m_commandKeys ? tr("Control") : tr("Ctrl"));
    if (modifiers & AltDown)
        parts << (m_commandKeys ? tr("Option") : tr("Alt"));
    if (modifiers & ShiftDown)
        parts << tr("Shift");
    if (modifiers & MetaDown)
        parts << (m_commandKeys ? tr("Command") : tr("Meta"));
    return parts.join(QStringLiteral(" + "));
}

QString HotCorners::stringProperty(const char *name) const
{
    return m_interface->isValid() ? m_interface->property(name).toString() : QString();
}

int HotCorners::intProperty(const char *name) const
{
    return m_interface->isValid() ? m_interface->property(name).toInt() : 0;
}
