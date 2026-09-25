#include "networkmockmodel.h"

#include <QDateTime>

namespace {
constexpr int activated = 2;
constexpr int deactivated = 4;
constexpr int availableConnection = 1;
constexpr int availableAccessPoint = 2;
constexpr int wiredType = 13;
constexpr int wirelessType = 14;
constexpr int noSecurity = 0;
constexpr int wpa2Psk = 7;
constexpr int sae = 9;
}

NetworkMockModel::NetworkMockModel(QObject *parent)
    : QAbstractListModel(parent)
{
    const QDateTime now = QDateTime::currentDateTime();

    m_items = {
        {
            { "name", QStringLiteral("Cutefish Studio") },
            { "connectionIcon", QStringLiteral("network-wireless-connected-100") },
            { "connectionState", activated },
            { "itemType", availableConnection },
            { "securityType", wpa2Psk },
            { "securityTypeString", QStringLiteral("WPA2 Personal") },
            { "signal", 92 },
            { "type", wirelessType },
            { "uuid", QStringLiteral("mock-wifi-connected") },
            { "ipAddress", QStringLiteral("192.168.1.86") },
            { "ipV6Address", QStringLiteral("fe80::1c2b:3aff:fe4d:5e6f") },
            { "gateWay", QStringLiteral("192.168.1.1") },
            { "nameServer", QStringLiteral("192.168.1.1") },
            { "macAddress", QStringLiteral("80:30:49:12:34:56") },
            { "timeStamp", now }
        },
        {
            { "name", QStringLiteral("Coffee Shop") },
            { "connectionIcon", QStringLiteral("network-wireless-connected-75") },
            { "connectionState", deactivated },
            { "itemType", availableAccessPoint },
            { "securityType", wpa2Psk },
            { "securityTypeString", QStringLiteral("WPA2 Personal") },
            { "signal", 74 },
            { "type", wirelessType },
            { "uuid", QStringLiteral("mock-wifi-saved") },
            { "timeStamp", now.addSecs(-3600) }
        },
        {
            { "name", QStringLiteral("Guest Network") },
            { "connectionIcon", QStringLiteral("network-wireless-connected-50") },
            { "connectionState", deactivated },
            { "itemType", availableAccessPoint },
            { "securityType", noSecurity },
            { "securityTypeString", QStringLiteral("None") },
            { "signal", 51 },
            { "type", wirelessType },
            { "timeStamp", now.addSecs(-7200) }
        },
        {
            { "name", QStringLiteral("Neighbor Wi-Fi") },
            { "connectionIcon", QStringLiteral("network-wireless-connected-25") },
            { "connectionState", deactivated },
            { "itemType", availableAccessPoint },
            { "securityType", sae },
            { "securityTypeString", QStringLiteral("WPA3 Personal") },
            { "signal", 28 },
            { "type", wirelessType },
            { "timeStamp", now.addSecs(-10800) }
        },
        {
            { "name", QStringLiteral("Wired connection 1") },
            { "connectionIcon", QStringLiteral("network-wired") },
            { "connectionState", activated },
            { "itemType", availableConnection },
            { "securityType", noSecurity },
            { "signal", 100 },
            { "type", wiredType },
            { "uuid", QStringLiteral("mock-wired-connected") },
            { "timeStamp", now }
        },
        {
            { "name", QStringLiteral("USB Ethernet") },
            { "connectionIcon", QStringLiteral("network-wired") },
            { "connectionState", deactivated },
            { "itemType", availableConnection },
            { "securityType", noSecurity },
            { "signal", 100 },
            { "type", wiredType },
            { "uuid", QStringLiteral("mock-wired-usb") },
            { "timeStamp", now.addSecs(-18000) }
        }
    };

    for (int i = 0; i < m_items.size(); ++i) {
        auto &item = m_items[i];
        const QString name = item.value("name").toString();
        item["itemUniqueName"] = name;
        item["ssid"] = name;
        item["connectionPath"] = QString();
        item["devicePath"] = QStringLiteral("/mock/device/%1").arg(i);
        item["specificPath"] = QStringLiteral("/mock/network/%1").arg(i);
        item["slave"] = false;
    }
}

int NetworkMockModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_items.size();
}

QVariant NetworkMockModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_items.size())
        return {};

    const QByteArray roleName = roleNames().value(role);
    return m_items.at(index.row()).value(QString::fromLatin1(roleName));
}

QHash<int, QByteArray> NetworkMockModel::roleNames() const
{
    return {
        { ConnectionDetailsRole, "connectionDetails" },
        { ConnectionIconRole, "connectionIcon" },
        { ConnectionPathRole, "connectionPath" },
        { ConnectionStateRole, "connectionState" },
        { DeviceNameRole, "deviceName" },
        { DevicePathRole, "devicePath" },
        { DeviceStateRole, "deviceState" },
        { DuplicateRole, "duplicate" },
        { ItemUniqueNameRole, "itemUniqueName" },
        { ItemTypeRole, "itemType" },
        { LastUsedRole, "lastUsed" },
        { LastUsedDateOnlyRole, "lastUsedDateOnly" },
        { NameRole, "name" },
        { SecurityTypeRole, "securityType" },
        { SecurityTypeStringRole, "securityTypeString" },
        { SectionRole, "section" },
        { SignalRole, "signal" },
        { SlaveRole, "slave" },
        { SsidRole, "ssid" },
        { SpecificPathRole, "specificPath" },
        { TimeStampRole, "timeStamp" },
        { TypeRole, "type" },
        { UniRole, "uni" },
        { UuidRole, "uuid" },
        { VpnStateRole, "vpnState" },
        { VpnTypeRole, "vpnType" },
        { RxBytesRole, "rxBytes" },
        { TxBytesRole, "txBytes" },
        { IpAddressRole, "ipAddress" },
        { IpV6AddressRole, "ipV6Address" },
        { GateWayRole, "gateWay" },
        { NameServerRole, "nameServer" },
        { MacAddressRole, "macAddress" }
    };
}
