#include "screenrecordingmodel.h"
#include "applicationregistry.h"

ScreenRecordingModel::ScreenRecordingModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_registry(ApplicationRegistry::instance())
{
    connect(&m_permissions, &ScreenRecordingPermissions::changed, this,
            &ScreenRecordingModel::reload);
    // The registry scans in the background and rescans on install or removal.
    auto *applications = m_registry->applications();
    connect(applications, &QAbstractItemModel::modelReset, this, &ScreenRecordingModel::reload);
    connect(applications, &QAbstractItemModel::rowsInserted, this, &ScreenRecordingModel::reload);
    connect(applications, &QAbstractItemModel::rowsRemoved, this, &ScreenRecordingModel::reload);
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
        const DesktopEntry *entry = m_registry->byId(grant.appId);
        if (!entry)
            continue;
        entries.append({grant.appId, entry->name().isEmpty() ? grant.appId : entry->name(),
                        entry->icon(), grant.allowed});
    }
    std::sort(entries.begin(), entries.end(), [](const Entry &a, const Entry &b) {
        return a.name.localeAwareCompare(b.name) < 0;
    });
    beginResetModel();
    m_entries = entries;
    endResetModel();
}
