#include "inputmethod.h"

#include "desktopentry.h"

#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QFile>
#include <QStandardPaths>

namespace {
constexpr auto service = "com.cutefish.InputMethod1";
constexpr auto path = "/com/cutefish/InputMethod1";

QVariantMap describe(const QString &id)
{
    QVariantMap method {{QStringLiteral("id"), id}, {QStringLiteral("name"), id}};
    const QString file = QStandardPaths::locate(QStandardPaths::ApplicationsLocation, id);
    QFile desktop(file);
    DesktopEntryData data;
    if (!file.isEmpty() && desktop.open(QIODevice::ReadOnly)
        && DesktopEntry::parse(id, file, desktop.readAll(), &data)) {
        if (!data.name.isEmpty())
            method[QStringLiteral("name")] = data.name;
        method[QStringLiteral("icon")] = data.icon;
    }
    return method;
}
} // namespace

InputMethod::InputMethod(QObject *parent)
    : QObject(parent)
{
    auto *watcher = new QDBusServiceWatcher(QLatin1String(service), QDBusConnection::sessionBus(),
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this,
            &InputMethod::refreshInterface);
    refreshInterface();
}

void InputMethod::refreshInterface()
{
    delete m_interface;
    m_interface = new QDBusInterface(QLatin1String(service), QLatin1String(path),
                                     QLatin1String(service), QDBusConnection::sessionBus(), this);
    if (m_interface->isValid()) {
        connect(m_interface, SIGNAL(inputMethodChanged()), this, SIGNAL(currentChanged()));
        connect(m_interface, SIGNAL(runningChanged()), this, SIGNAL(runningChanged()));
    }
    emit availableChanged();
    emit currentChanged();
    emit runningChanged();
    refreshMethods();
}

void InputMethod::refreshMethods()
{
    m_methods.clear();
    if (m_interface->isValid()) {
        const QDBusReply<QStringList> reply =
                m_interface->call(QStringLiteral("availableInputMethods"));
        for (const auto &id : reply.value())
            m_methods << describe(id);
    }
    emit methodsChanged();
}

bool InputMethod::available() const
{
    return m_interface->isValid();
}

QString InputMethod::current() const
{
    return m_interface->isValid() ? m_interface->property("inputMethod").toString() : QString();
}

bool InputMethod::running() const
{
    return m_interface->isValid() && m_interface->property("running").toBool();
}

void InputMethod::select(const QString &id)
{
    if (!m_interface->isValid())
        return;
    auto *watcher = new QDBusPendingCallWatcher(
            m_interface->asyncCall(QStringLiteral("setInputMethod"), id), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, id](QDBusPendingCallWatcher *call) {
                const QDBusPendingReply<bool> reply = *call;
                if (reply.isError() || !reply.value()) {
                    qWarning() << "Cannot select input method" << id << reply.error().message();
                    refreshMethods();
                }
                emit currentChanged();
                call->deleteLater();
            });
}
