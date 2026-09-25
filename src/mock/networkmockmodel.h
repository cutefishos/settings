#pragma once

#include <QAbstractListModel>
#include <QVariantMap>

class NetworkMockModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit NetworkMockModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    enum Role {
        ConnectionDetailsRole = Qt::UserRole + 1,
        ConnectionIconRole,
        ConnectionPathRole,
        ConnectionStateRole,
        DeviceNameRole,
        DevicePathRole,
        DeviceStateRole,
        DuplicateRole,
        ItemUniqueNameRole,
        ItemTypeRole,
        LastUsedRole,
        LastUsedDateOnlyRole,
        NameRole,
        SecurityTypeRole,
        SecurityTypeStringRole,
        SectionRole,
        SignalRole,
        SlaveRole,
        SsidRole,
        SpecificPathRole,
        TimeStampRole,
        TypeRole,
        UniRole,
        UuidRole,
        VpnStateRole,
        VpnTypeRole,
        RxBytesRole,
        TxBytesRole,
        IpAddressRole,
        IpV6AddressRole,
        GateWayRole,
        NameServerRole,
        MacAddressRole
    };

    QList<QVariantMap> m_items;
};
