#include "keyboard.h"

#include <QCollator>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QFile>
#include <QXmlStreamReader>

#include <algorithm>
#include <libintl.h>

namespace {
constexpr auto service = "com.cutefish.Keyboard1";
constexpr auto path = "/com/cutefish/Keyboard1";
constexpr auto rulesFile = "/usr/share/X11/xkb/rules/evdev.xml";

QString translate(const QString &description)
{
    return QString::fromUtf8(dgettext("xkeyboard-config", description.toUtf8().constData()));
}

QVariantList readLayouts()
{
    QFile file(QString::fromLatin1(rulesFile));
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Cannot read keyboard layouts from" << file.fileName();
        return {};
    }
    bind_textdomain_codeset("xkeyboard-config", "UTF-8");

    QVariantList layouts;
    QXmlStreamReader xml(&file);
    bool inLayoutList = false;
    bool inVariant = false;
    QString layout;
    QString name;
    QString description;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            if (xml.name() == u"layoutList") {
                inLayoutList = true;
            } else if (!inLayoutList) {
                continue;
            } else if (xml.name() == u"variant") {
                inVariant = true;
            } else if (xml.name() == u"configItem") {
                name.clear();
                description.clear();
            } else if (xml.name() == u"name") {
                name = xml.readElementText();
            } else if (xml.name() == u"description") {
                description = xml.readElementText();
            }
        } else if (xml.isEndElement() && inLayoutList) {
            if (xml.name() == u"layoutList") {
                inLayoutList = false;
            } else if (xml.name() == u"variant") {
                inVariant = false;
            } else if (xml.name() == u"configItem" && !name.isEmpty()) {
                if (!inVariant)
                    layout = name;
                // A placeholder for a layout file the user writes by hand.
                if (layout == QLatin1String("custom"))
                    continue;
                const QString id = inVariant ? layout + QLatin1Char('-') + name : layout;
                layouts << QVariantMap {{QStringLiteral("id"), id},
                                        {QStringLiteral("name"), translate(description)}};
            }
        }
    }
    if (xml.hasError())
        qWarning() << "Cannot parse" << file.fileName() << xml.errorString();

    QCollator collator;
    collator.setCaseSensitivity(Qt::CaseInsensitive);
    std::sort(layouts.begin(), layouts.end(), [&collator](const QVariant &a, const QVariant &b) {
        return collator.compare(a.toMap().value(QStringLiteral("name")).toString(),
                                b.toMap().value(QStringLiteral("name")).toString())
                < 0;
    });
    return layouts;
}
} // namespace

Keyboard::Keyboard(QObject *parent)
    : QObject(parent)
{
    auto *watcher = new QDBusServiceWatcher(QLatin1String(service), QDBusConnection::sessionBus(),
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &Keyboard::refreshInterface);
    refreshInterface();
}

void Keyboard::refreshInterface()
{
    delete m_interface;
    m_interface = new QDBusInterface(QLatin1String(service), QLatin1String(path),
                                     QLatin1String(service), QDBusConnection::sessionBus(), this);
    if (m_interface->isValid()) {
        connect(m_interface, SIGNAL(repeatEnabledChanged()), this, SIGNAL(repeatEnabledChanged()));
        connect(m_interface, SIGNAL(repeatDelayChanged()), this, SIGNAL(repeatDelayChanged()));
        connect(m_interface, SIGNAL(repeatIntervalChanged()), this,
                SIGNAL(repeatIntervalChanged()));
        connect(m_interface, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
    }
    notifyChanged();
}

void Keyboard::notifyChanged()
{
    emit availableChanged();
    emit repeatEnabledChanged();
    emit repeatDelayChanged();
    emit repeatIntervalChanged();
    emit layoutChanged();
}

void Keyboard::call(const QString &method, const QVariant &value)
{
    if (!m_interface->isValid())
        return;
    auto *watcher = new QDBusPendingCallWatcher(m_interface->asyncCall(method, value), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, method](QDBusPendingCallWatcher *reply) {
                const QDBusPendingReply<> result = *reply;
                if (result.isError()) {
                    qWarning() << "Cannot update keyboard setting" << method
                               << result.error().message();
                    notifyChanged();
                }
                reply->deleteLater();
            });
}

bool Keyboard::available() const
{
    return m_interface->isValid();
}

bool Keyboard::repeatEnabled() const
{
    return m_interface->isValid() ? m_interface->property("repeatEnabled").toBool() : true;
}

void Keyboard::setRepeatEnabled(bool enabled)
{
    call(QStringLiteral("setRepeatEnabled"), enabled);
}

int Keyboard::repeatDelay() const
{
    return m_interface->property("repeatDelay").toInt();
}

void Keyboard::setRepeatDelay(int milliseconds)
{
    call(QStringLiteral("setRepeatDelay"), milliseconds);
}

int Keyboard::repeatInterval() const
{
    return m_interface->property("repeatInterval").toInt();
}

void Keyboard::setRepeatInterval(int milliseconds)
{
    call(QStringLiteral("setRepeatInterval"), milliseconds);
}

QString Keyboard::layout() const
{
    return m_interface->property("layout").toString();
}

QString Keyboard::layoutName() const
{
    const QString id = layout();
    for (const auto &entry : layouts()) {
        const QVariantMap map = entry.toMap();
        if (map.value(QStringLiteral("id")).toString() == id)
            return map.value(QStringLiteral("name")).toString();
    }
    return id;
}

QVariantList Keyboard::layouts() const
{
    if (m_layouts.isEmpty())
        m_layouts = readLayouts();
    return m_layouts;
}

void Keyboard::selectLayout(const QString &id)
{
    if (!m_interface->isValid())
        return;
    auto *watcher = new QDBusPendingCallWatcher(
            m_interface->asyncCall(QStringLiteral("setLayout"), id), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this, id](QDBusPendingCallWatcher *call) {
                const QDBusPendingReply<bool> reply = *call;
                if (reply.isError() || !reply.value())
                    qWarning() << "Cannot select keyboard layout" << id << reply.error().message();
                emit layoutChanged();
                call->deleteLater();
            });
}
