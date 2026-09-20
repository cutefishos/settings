#pragma once

#include <QAbstractListModel>
#include <QFileSystemWatcher>

#include "screenrecordingpermissions.h"

// Installed applications that have asked to record the screen, with their
// grant state. A record whose desktop entry is gone stays in the store but is
// not listed; it comes back with the application.
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
    QFileSystemWatcher m_applications;
    QList<Entry> m_entries;
};
