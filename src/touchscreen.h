#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <QDBusInterface>
#include <QObject>
#include <QStringList>

class Touchscreen : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY devicesChanged)
    Q_PROPERTY(QStringList devices READ devices NOTIFY devicesChanged)
    Q_PROPERTY(QStringList disabledDevices READ disabledDevices NOTIFY disabledDevicesChanged)

public:
    explicit Touchscreen(QObject *parent = nullptr);

    bool available() const;
    QStringList devices() const;
    QStringList disabledDevices() const;

    Q_INVOKABLE void setDeviceEnabled(const QString &name, bool enabled);

signals:
    void devicesChanged();
    void disabledDevicesChanged();

private:
    void refreshInterface();
    void notifyChanged();
    QStringList list(const char *name) const;
    QDBusInterface *m_interface = nullptr;
};

#endif // TOUCHSCREEN_H
