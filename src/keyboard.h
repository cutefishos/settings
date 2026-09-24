#pragma once

#include <QDBusInterface>
#include <QObject>
#include <QVariantList>

// Client of the compositor's com.cutefish.Keyboard1 service.
class Keyboard : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool repeatEnabled READ repeatEnabled WRITE setRepeatEnabled NOTIFY
                       repeatEnabledChanged)
    Q_PROPERTY(int repeatDelay READ repeatDelay WRITE setRepeatDelay NOTIFY repeatDelayChanged)
    Q_PROPERTY(int repeatInterval READ repeatInterval WRITE setRepeatInterval NOTIFY
                       repeatIntervalChanged)
    // "layout" or "layout-variant", as xkeyboard-config names them.
    Q_PROPERTY(QString layout READ layout NOTIFY layoutChanged)
    Q_PROPERTY(QString layoutName READ layoutName NOTIFY layoutChanged)
    // Every layout and variant as {id, name}, sorted by name. Read on first use.
    Q_PROPERTY(QVariantList layouts READ layouts CONSTANT)

public:
    explicit Keyboard(QObject *parent = nullptr);

    bool available() const;

    bool repeatEnabled() const;
    void setRepeatEnabled(bool enabled);

    int repeatDelay() const;
    void setRepeatDelay(int milliseconds);

    int repeatInterval() const;
    void setRepeatInterval(int milliseconds);

    QString layout() const;
    QString layoutName() const;

    QVariantList layouts() const;

    Q_INVOKABLE void selectLayout(const QString &id);

signals:
    void availableChanged();
    void repeatEnabledChanged();
    void repeatDelayChanged();
    void repeatIntervalChanged();
    void layoutChanged();

private:
    void refreshInterface();
    void notifyChanged();
    void call(const QString &method, const QVariant &value);
    QDBusInterface *m_interface = nullptr;
    mutable QVariantList m_layouts;
};
