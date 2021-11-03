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

private slots:
    void onInitJobResult(BluezQt::InitManagerJob *job);
    void operationalChanged(bool operational);

private:
    BluezQt::Manager *m_manager;
    BluetoothAgent *m_agent;
    BluezQt::AdapterPtr m_adapter;
    QString m_name;
};

#endif // BLUETOOTHMANAGER_H
