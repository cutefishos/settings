#ifndef BLUETOOTHMANAGER_H
#define BLUETOOTHMANAGER_H

#include <QObject>
#include <BluezQt/Manager>
#include "bluetoothagent.h"

class BluetoothManager : public QObject
{
    Q_OBJECT

public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager();

    Q_INVOKABLE void setName(const QString &name);
    Q_INVOKABLE void connectToDevice(const QString address);
    Q_INVOKABLE void requestParingConnection(const QString address);
    Q_INVOKABLE void confirmMatchButton(const bool match);
    Q_INVOKABLE void deviceDisconnect(const QString address);
    Q_INVOKABLE void deviceRemoved(const QString address);
    Q_INVOKABLE void stopMediaPlayer(const QString address);

signals:
    void showPairDialog(const QString name, const QString pin);

private slots:
    void onInitJobResult(BluezQt::InitManagerJob *job);
    void operationalChanged(bool operational);
    void confirmationRequested(const QString &passkey, const BluezQt::Request<> &req);

private:
    BluezQt::Manager *m_manager;
    BluetoothAgent *m_agent;
    BluezQt::AdapterPtr m_adapter;
    BluezQt::DevicePtr m_device;
    BluezQt::Request<> m_req;
    QString m_name;
};

#endif // BLUETOOTHMANAGER_H
