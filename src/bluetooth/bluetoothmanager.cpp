#include "bluetoothmanager.h"
#include <QDebug>

#include <BluezQt/InitManagerJob>
#include <BluezQt/Adapter>

BluetoothManager::BluetoothManager(QObject *parent)
    : QObject(parent)
    , m_agent(new BluetoothAgent(this))
{
    m_manager = new BluezQt::Manager(this);
    BluezQt::InitManagerJob *initJob = m_manager->init();
    initJob->start();
    connect(initJob, &BluezQt::InitManagerJob::result, this, &BluetoothManager::onInitJobResult);

    connect(m_manager, &BluezQt::Manager::bluetoothBlockedChanged, this, [=] (bool blocked) {
        if (!blocked) {
            BluezQt::AdapterPtr adaptor = m_manager->adapters().first();
            if (adaptor) {
                if (!adaptor->isDiscoverable()) {
                    adaptor->setDiscoverable(true);
                }
            }
        }
    });
}

BluetoothManager::~BluetoothManager()
{
    m_manager->unregisterAgent(m_agent);

    delete m_agent;
    delete m_manager;
}

void BluetoothManager::setName(const QString &name)
{
    BluezQt::AdapterPtr adaptor = m_manager->usableAdapter();
    adaptor->setName(name);
}

void BluetoothManager::onInitJobResult(BluezQt::InitManagerJob *job)
{
    if (job->error()) {
        qDebug() << "Init Bluetooth error";
        return;
    }

    // Make sure to register agent when bluetoothd starts
    operationalChanged(m_manager->isOperational());
    connect(m_manager, &BluezQt::Manager::operationalChanged, this, &BluetoothManager::operationalChanged);

    m_adapter = m_manager->usableAdapter();
    if (m_adapter) {
        setName("CutefishOS");

        if (!m_adapter->isDiscoverable())
            m_adapter->startDiscovery();
    }
}

void BluetoothManager::operationalChanged(bool operational)
{
    if (operational) {
        m_manager->registerAgent(m_agent);
    } else {
        // Attempt to start bluetoothd
        BluezQt::Manager::startService();
    }
}
