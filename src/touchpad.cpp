#include "touchpad.h"

Touchpad::Touchpad(QObject *parent)
    : QObject(parent)
    , m_iface("com.cutefish.Settings",
              "/Touchpad",
              "com.cutefish.Touchpad",
              QDBusConnection::sessionBus(), this)
{

}

bool Touchpad::available() const
{
    if (!m_iface.isValid())
        return false;

    return m_iface.property("available").toBool();
}

bool Touchpad::enabled() const
{
    return m_iface.property("enabled").toBool();
}

void Touchpad::setEnabled(bool enabled)
{
    m_iface.asyncCall("setEnabled", enabled);
    emit enabledChanged();
}

bool Touchpad::tapToClick() const
{
    return m_iface.property("tapToClick").toBool();
}

void Touchpad::setTapToClick(bool enabled)
{
    m_iface.asyncCall("setTapToClick", enabled);
    emit tapToClickChanged();
}

qreal Touchpad::pointerAcceleration() const
{
    if (!m_iface.isValid())
        return 0;

    return m_iface.property("pointerAcceleration").toReal();
}

void Touchpad::setPointerAcceleration(qreal value)
{
    m_iface.asyncCall("setPointerAcceleration", value);
    emit pointerAccelerationChanged();
}
