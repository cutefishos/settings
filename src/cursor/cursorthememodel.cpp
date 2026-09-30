/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cursorthememodel.h"
#include "cursortheme.h"
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <preferences.h>

CursorThemeModel::CursorThemeModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_settings(Preferences::file(QStringLiteral("services")), QSettings::IniFormat)
{
    m_settings.beginGroup(QStringLiteral("Appearance"));
    initThemes();

    m_currentTheme = m_settings.value("CursorTheme", "default").toString();
    auto bus = QDBusConnection::sessionBus();
    for (const auto *signal : {"cursorThemeChanged", "cursorSizeChanged"})
        bus.connect("com.cutefish.Services", "/com/cutefish/Services/Appearance",
                    "com.cutefish.Services.Appearance", QLatin1String(signal), this,
                    SLOT(reload()));
    auto *watcher = new QDBusServiceWatcher("com.cutefish.Services", bus,
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &CursorThemeModel::reload);
}

CursorThemeModel::~CursorThemeModel()
{
    qDeleteAll(m_list);
    m_list.clear();
}

QHash<int, QByteArray> CursorThemeModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[CursorThemeModel::NameRole] = "name";
    roleNames[CursorThemeModel::ImageRole] = "image";
    roleNames[CursorThemeModel::PathRole] = "path";
    roleNames[CursorThemeModel::IdRole] = "id";
    return roleNames;
}

int CursorThemeModel::rowCount(const QModelIndex &) const
{
    return m_list.size();
}

QVariant CursorThemeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_list.size())
        return QVariant();

    CursorTheme *theme = m_list.at(index.row());

    switch (role) {
    case CursorThemeModel::NameRole:
        return theme->name();
        break;
    case CursorThemeModel::ImageRole:
        return theme->pixmap();
        break;
    case CursorThemeModel::PathRole:
        return theme->path();
        break;
    case CursorThemeModel::IdRole:
        return theme->id();
        break;
    default:
        break;
    }

    return QVariant();
}

QString CursorThemeModel::currentTheme() const
{
    return m_currentTheme;
}

void CursorThemeModel::setCurrentTheme(const QString &theme)
{
    if (m_currentTheme != theme && themeIndex(theme) >= 0)
        apply(QStringLiteral("setCursorTheme"), theme);
}

int CursorThemeModel::cursorSize() const
{
    return qBound(16, m_settings.value("CursorSize", 24).toInt(), 64);
}

void CursorThemeModel::setCursorSize(int size)
{
    apply(QStringLiteral("setCursorSize"), qBound(16, size, 64));
}

void CursorThemeModel::reload()
{
    m_settings.sync();
    m_currentTheme = m_settings.value("CursorTheme", "default").toString();
    emit currentThemeChanged();
    emit cursorSizeChanged();
}

void CursorThemeModel::apply(const QString &method, const QVariant &value)
{
    auto message = QDBusMessage::createMethodCall("com.cutefish.Services",
                                                  "/com/cutefish/Services/Appearance",
                                                  "com.cutefish.Services.Appearance", method);
    message << value;
    auto *watcher =
            new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *call) {
                const QDBusPendingReply<> reply = *call;
                if (reply.isError())
                    qWarning() << "Cannot apply cursor theme:" << reply.error().message();
                reload();
                call->deleteLater();
            });
}

int CursorThemeModel::themeIndex(const QString &theme)
{
    QString id = theme;

    if (id == "default") {
        id = m_defaultTheme;
    }

    for (int i = 0; i < m_list.size(); ++i) {
        const CursorTheme *t = m_list.at(i);
        if (t->id() == id)
            return i;
    }

    return -1;
}

void CursorThemeModel::initThemes()
{
    for (const QString &baseDir : paths()) {
        QDir dir(baseDir);
        if (!dir.exists())
            continue;

        for (const QString &name : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (!dir.cd(name))
                continue;

            processDir(dir);
            dir.cdUp();
        }
    }
}

QStringList CursorThemeModel::paths()
{
    QStringList list;
    list = qEnvironmentVariable("XCURSOR_PATH").split(':', Qt::SkipEmptyParts);
    if (!list.isEmpty()) {
        for (auto &path : list)
            if (path.startsWith("~/"))
                path.replace(0, 1, QDir::homePath());
        return list;
    }
    list << QDir::homePath() + "/.icons";
    for (const auto &path : QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))
        list << path + "/icons";
    list << "/usr/share/pixmaps";
    list.removeDuplicates();
    return list;
}

void CursorThemeModel::processDir(const QDir &dir)
{
    if (m_defaultTheme.isNull() && dir.dirName() == "default") {
        QFileInfo info(dir.path());

        // Dir are link
        if (info.isSymLink()) {
            QFileInfo target(info.symLinkTarget());
            if (target.exists() && (target.isDir() || target.isSymLink()))
                m_defaultTheme = target.fileName();

            return;
        }

        if (!dir.exists("cursors")
            || QDir(dir.path() + "/cursors")
                       .entryList(QDir::Files | QDir::NoDotAndDotDot)
                       .isEmpty()) {
            if (dir.exists("index.theme")) {
                CursorTheme theme(dir);
                m_defaultTheme = theme.inherits();
                return;
            }
        }
    }

    if (!dir.exists("cursors"))
        return;

    if (!dir.exists("index.theme") && !dir.exists("cursor.theme"))
        return;

    CursorTheme *theme = new CursorTheme(dir);

    for (int i = 0; i < m_list.size(); ++i) {
        CursorTheme *t = m_list.at(i);
        if (t->id() == theme->id()) {
            delete theme;
            return;
        }
    }

    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(theme);
    endInsertRows();
}
