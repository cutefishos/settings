// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "brightness.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusReply>

namespace {
const QString Service = QStringLiteral("com.cutefish.Services");
const QString Path = QStringLiteral("/com/cutefish/Services/Brightness");
const QString Interface = QStringLiteral("com.cutefish.Services.Brightness");
} // namespace

Brightness::Brightness(QObject *parent)
    : QObject(parent)
    , m_iface(Service, Path, Interface, QDBusConnection::sessionBus())
{
    QDBusConnection bus = QDBusConnection::sessionBus();
    bus.connect(Service, Path, Interface, QStringLiteral("displaysChanged"), this, SLOT(refresh()));
    bus.connect(Service, Path, Interface, QStringLiteral("displayBrightnessChanged"), this,
                SLOT(handleValueChanged(QString, int)));
}

QString Brightness::display() const
{
    return m_display;
}

void Brightness::setDisplay(const QString &display)
{
    if (m_display == display)
        return;
    m_display = display;
    emit displayChanged();
    refresh();
}

bool Brightness::available() const
{
    return m_available;
}

int Brightness::value() const
{
    return m_value;
}

void Brightness::setValue(int value)
{
    if (m_available)
        m_iface.asyncCall(QStringLiteral("setDisplayBrightness"), m_display, value);
}

void Brightness::refresh()
{
    const QDBusReply<QStringList> displays = m_iface.call(QStringLiteral("displays"));
    const bool available =
            !m_display.isEmpty() && displays.isValid() && displays.value().contains(m_display);
    if (available) {
        const QDBusReply<int> value = m_iface.call(QStringLiteral("displayBrightness"), m_display);
        if (value.isValid())
            handleValueChanged(m_display, value.value());
    }
    if (m_available != available) {
        m_available = available;
        emit availableChanged();
    }
}

void Brightness::handleValueChanged(const QString &display, int value)
{
    if (display != m_display || value < 0 || m_value == value)
        return;
    m_value = value;
    emit valueChanged();
}
