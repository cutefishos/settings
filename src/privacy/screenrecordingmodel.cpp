#include "screenrecordingmodel.h"
#include "desktopentry.h"

#include <QFile>
#include <QStandardPaths>

ScreenRecordingModel::ScreenRecordingModel(QObject *parent)
    : QAbstractListModel(parent)
{
    connect(&m_permissions, &ScreenRecordingPermissions::changed, this,
            &ScreenRecordingModel::reload);
    reload();
}

int ScreenRecordingModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant ScreenRecordingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_entries.size())
        return {};
    const Entry &entry = m_entries.at(index.row());
    switch (role) {
    case AppIdRole:
        return entry.appId;
    case NameRole:
        return entry.name;
    case IconRole:
        return entry.icon;
    case AllowedRole:
        return entry.allowed;
    default:
        return {};
    }
}

QHash<int, QByteArray> ScreenRecordingModel::roleNames() const
{
    return {{AppIdRole, "appId"}, {NameRole, "name"}, {IconRole, "icon"}, {AllowedRole, "allowed"}};
}

void ScreenRecordingModel::setAllowed(int row, bool allowed)
{
    if (row < 0 || row >= m_entries.size())
        return;
    m_permissions.setAllowed(m_entries.at(row).appId, allowed);
    // The file watcher reloads too, but the switch must not snap back meanwhile.
    m_entries[row].allowed = allowed;
    Q_EMIT dataChanged(index(row), index(row), {AllowedRole});
}

void ScreenRecordingModel::remove(int row)
{
    if (row < 0 || row >= m_entries.size())
        return;
    m_permissions.remove(m_entries.at(row).appId);
    reload();
}

void ScreenRecordingModel::reload()
{
    QList<Entry> entries;
    for (const auto &grant : m_permissions.grants()) {
        Entry entry {grant.appId, grant.appId, QString(), grant.allowed};
        const QString path = QStandardPaths::locate(QStandardPaths::ApplicationsLocation,
                                                    grant.appId + QStringLiteral(".desktop"));
        QFile file(path);
        DesktopEntryData data;
        if (!path.isEmpty() && file.open(QIODevice::ReadOnly)
            && DesktopEntry::parse(grant.appId, path, file.readAll(), &data)) {
            if (!data.name.isEmpty())
                entry.name = data.name;
            entry.icon = data.icon;
        }
        entries.append(entry);
    }
    std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
        return a.name.localeAwareCompare(b.name) < 0;
    });
    beginResetModel();
    m_entries = entries;
    endResetModel();
}
