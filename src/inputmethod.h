#pragma once

#include <QDBusInterface>
#include <QObject>
#include <QVariantList>

// Client of the compositor's com.cutefish.InputMethod1 service.
class InputMethod : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(QVariantList methods READ methods NOTIFY methodsChanged)
    Q_PROPERTY(QString current READ current NOTIFY currentChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit InputMethod(QObject *parent = nullptr);

    bool available() const;

    QVariantList methods() const { return m_methods; }

    QString current() const;
    bool running() const;

    // An empty id turns the input method off.
    Q_INVOKABLE void select(const QString &id);

signals:
    void availableChanged();
    void methodsChanged();
    void currentChanged();
    void runningChanged();

private:
    void refreshInterface();
    void refreshMethods();
    QDBusInterface *m_interface = nullptr;
    QVariantList m_methods;
};
