#pragma once

#include <QAbstractListModel>

#include "screenrecordingpermissions.h"

class ApplicationRegistry;

// Installed applications that have asked to record the screen, with their
// grant state. Which applications exist is the registry's knowledge: a record
// whose entry is gone stays in the store but is not listed, and comes back
// with the application.
class ScreenRecordingModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        AppIdRole = Qt::UserRole + 1,
        NameRole,
        IconRole,
        AllowedRole
    };

    explicit ScreenRecordingModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void setAllowed(int row, bool allowed);
    Q_INVOKABLE void remove(int row);

private:
    struct Entry {
        QString appId;
        QString name;
        QString icon;
        bool allowed = false;
    };

    void reload();

    ScreenRecordingPermissions m_permissions;
    ApplicationRegistry *m_registry;
    QList<Entry> m_entries;
};
