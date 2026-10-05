#include "touchscreen.h"
#include <QDBusPendingCall>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDebug>

static constexpr auto service = "com.cutefish.Touchscreen1";
static constexpr auto path = "/com/cutefish/Touchscreen1";

Touchscreen::Touchscreen(QObject *parent)
    : QObject(parent)
{
    auto *watcher = new QDBusServiceWatcher(QLatin1String(service), QDBusConnection::sessionBus(),
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this,
            &Touchscreen::refreshInterface);
    refreshInterface();
}

void Touchscreen::refreshInterface()
{
    delete m_interface;
    m_interface = new QDBusInterface(QLatin1String(service), QLatin1String(path),
                                     QLatin1String(service), QDBusConnection::sessionBus(), this);
    if (m_interface->isValid()) {
        connect(m_interface, SIGNAL(devicesChanged()), this, SIGNAL(devicesChanged()));
        connect(m_interface, SIGNAL(disabledDevicesChanged()), this,
                SIGNAL(disabledDevicesChanged()));
    }
    notifyChanged();
}

void Touchscreen::notifyChanged()
{
    emit devicesChanged();
    emit disabledDevicesChanged();
}

QStringList Touchscreen::list(const char *name) const
{
    return m_interface->isValid() ? m_interface->property(name).toStringList() : QStringList();
}

bool Touchscreen::available() const
{
    return !devices().isEmpty();
}

QStringList Touchscreen::devices() const
{
    return list("devices");
}

QStringList Touchscreen::disabledDevices() const
{
    return list("disabledDevices");
}

void Touchscreen::setDeviceEnabled(const QString &name, bool enabled)
{
    if (!m_interface->isValid())
        return;
    auto *watcher = new QDBusPendingCallWatcher(
            m_interface->asyncCall(QStringLiteral("setDeviceEnabled"), name, enabled), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *reply) {
                const QDBusPendingReply<> result = *reply;
                if (result.isError()) {
                    qWarning() << "Cannot update touchscreen setting" << result.error().message();
                    notifyChanged();
                }
                reply->deleteLater();
            });
}
