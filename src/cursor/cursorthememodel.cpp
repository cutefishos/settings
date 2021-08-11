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
#include <QDBusInterface>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

CursorThemeModel::CursorThemeModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_settings("cutefishos", "theme")
{
    initThemes();

    m_currentTheme = m_settings.value("CursorTheme", "default").toString();
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
    if (m_currentTheme != theme) {
        m_currentTheme = theme;
        emit currentThemeChanged();

        QDBusInterface interface("org.cutefish.Settings",
                                 "/Theme",
                                 "org.cutefish.Theme",
                                 QDBusConnection::sessionBus());
        if (interface.isValid())
            interface.asyncCall("setCursorTheme", m_currentTheme);
    }
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
    list << QDir::home().path() + ".icons";
    list << "/usr/share/icons";
    list << "/usr/share/pixmaps";
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

        if (!dir.exists("cursors") || QDir(dir.path() + "/cursor").entryList(QDir::Files | QDir::NoDotAndDotDot).isEmpty()) {
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
        if (t->name() == theme->name()) {
            delete theme;
            return;
        }
    }

    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.append(theme);
    endInsertRows();
}
