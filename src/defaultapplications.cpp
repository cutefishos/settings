/*
 * Copyright (C) 2021 - 2022 CutefishOS Team.
 *
 * Author:     Kate Leet <kate@cutefishos.com>
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

#include "defaultapplications.h"
#include "applicationregistry.h"
#include "desktopentry.h"

#include <QFileInfo>
#include <QSettings>
#include <QStandardPaths>
#include <QDebug>

static QString firstDesktopEntry(const QString &value)
{
    return value.split(QLatin1Char(';'), Qt::SkipEmptyParts).value(0).trimmed();
}

DefaultApplications::DefaultApplications(QObject *parent)
    : QObject(parent)
{
    ApplicationRegistry *registry = ApplicationRegistry::instance();
    connect(registry, &ApplicationRegistry::applicationsChanged,
            this, &DefaultApplications::loadApps);
    if (!registry->loading())
        loadApps();
}

void DefaultApplications::loadApps()
{
    m_browserList.clear();
    m_fileManagerList.clear();
    m_emailList.clear();
    m_terminalList.clear();
    m_browserIndex = -1;
    m_fileManagerIndex = -1;
    m_emailIndex = -1;
    m_terminalIndex = -1;

    for (DesktopEntry *entry : ApplicationRegistry::instance()->entries()) {
        if (entry->hidden())
            continue;

        AppItem item;
        item.path = entry->path();
        item.name = entry->name();
        item.icon = entry->icon();
        item.mimeType = entry->mimeTypes().join(QLatin1Char(';'));
        item.categories = entry->categories().join(QLatin1Char(';'));
        item.fileName = QFileInfo(entry->path()).fileName();

        if (entry->categories().contains(QStringLiteral("FileManager"))
                && entry->mimeTypes().contains(QStringLiteral("inode/directory"))) {
            m_fileManagerList.append(item);
        } else if (entry->categories().contains(QStringLiteral("WebBrowser"))
                   && entry->mimeTypes().contains(QStringLiteral("x-scheme-handler/http"))) {
            m_browserList.append(item);
        } else if (entry->categories().contains(QStringLiteral("Email"))
                   && entry->mimeTypes().contains(QStringLiteral("x-scheme-handler/mailto"))) {
            m_emailList.append(item);
        } else if (entry->categories().contains(QStringLiteral("TerminalEmulator"))) {
            m_terminalList.append(item);
        }
    }

    // XDG values are semicolon-separated lists and may end with a semicolon.
    QSettings mimeApps(mimeAppsListFilePath(), QSettings::IniFormat);
    mimeApps.beginGroup("Default Applications");

    QSettings settings("cutefishos", "defaultApps");

    const QString defaultBrowser = firstDesktopEntry(mimeApps.value("x-scheme-handler/http").toString());
    const QString defaultFM = firstDesktopEntry(mimeApps.value("inode/directory").toString());
    const QString defaultEMail = firstDesktopEntry(mimeApps.value("x-scheme-handler/mailto").toString());
    QString defaultTerminal = settings.value("terminal").toString();

    // Init indexes.
    for (int i = 0; i < m_browserList.size(); ++i) {
        if (defaultBrowser == m_browserList.at(i).fileName) {
            m_browserIndex = i;
            break;
        }
    }

    for (int i = 0; i < m_fileManagerList.size(); ++i) {
        if (defaultFM == m_fileManagerList.at(i).fileName) {
            m_fileManagerIndex = i;
            break;
        }
    }

    for (int i = 0; i < m_emailList.size(); ++i) {
        if (defaultEMail == m_emailList.at(i).fileName) {
            m_emailIndex = i;
            break;
        }
    }

    for (int i = 0; i < m_terminalList.size(); ++i) {
        if (defaultTerminal == m_terminalList.at(i).fileName) {
            m_terminalIndex = i;
            break;
        }
    }

    emit loadFinished();
}

QVariantList DefaultApplications::browserList()
{
    QVariantList list;

    for (const AppItem &item : m_browserList) {
        QVariantMap map;
        map["name"] = item.name;
        map["icon"] = item.icon;
        map["path"] = item.path;
        list << map;
    }

    return list;
}

QVariantList DefaultApplications::fileManagerList()
{
    QVariantList list;

    for (const AppItem &item : m_fileManagerList) {
        QVariantMap map;
        map["name"] = item.name;
        map["icon"] = item.icon;
        map["path"] = item.path;
        list << map;
    }

    return list;
}

QVariantList DefaultApplications::emailList()
{
    QVariantList list;

    for (const AppItem &item : m_emailList) {
        QVariantMap map;
        map["name"] = item.name;
        map["icon"] = item.icon;
        map["path"] = item.path;
        list << map;
    }

    return list;
}

QVariantList DefaultApplications::terminalList()
{
    QVariantList list;

    for (const AppItem &item : m_terminalList) {
        QVariantMap map;
        map["name"] = item.name;
        map["icon"] = item.icon;
        map["path"] = item.path;
        list << map;
    }

    return list;
}

int DefaultApplications::browserIndex()
{
    return m_browserIndex;
}

int DefaultApplications::fileManagerIndex()
{
    return m_fileManagerIndex;
}

int DefaultApplications::emailIndex()
{
    return m_emailIndex;
}

int DefaultApplications::terminalIndex()
{
    return m_terminalIndex;
}

void DefaultApplications::setDefaultBrowser(int index)
{
    if (index < 0 || index >= m_browserList.size())
        return;

    const QString desktop = m_browserList.at(index).fileName;

    setDefaultApp("x-scheme-handler/http", desktop);
    setDefaultApp("x-scheme-handler/https", desktop);
}

void DefaultApplications::setDefaultFileManager(int index)
{
    if (index < 0 || index >= m_fileManagerList.size())
        return;

    const QString desktop = m_fileManagerList.at(index).fileName;

    setDefaultApp("inode/directory", desktop);
}

void DefaultApplications::setDefaultEMail(int index)
{
    if (index < 0 || index >= m_emailList.size())
        return;

    const QString desktop = m_emailList.at(index).fileName;

    setDefaultApp("x-scheme-handler/mailto", desktop);
}

void DefaultApplications::setDefaultTerminal(int index)
{
    if (index < 0 || index >= m_terminalList.size())
        return;

    const QString desktop = m_terminalList.at(index).fileName;

    qDebug() << index << desktop;

    QSettings settings("cutefishos", "defaultApps");
    settings.setValue("terminal", desktop);
}

void DefaultApplications::setDefaultApp(const QString &mimeType, const QString &path)
{
    QSettings mimeApps(mimeAppsListFilePath(), QSettings::IniFormat);
    mimeApps.beginGroup("Default Applications");
    mimeApps.setValue(mimeType, path + QLatin1Char(';'));
}

QString DefaultApplications::mimeAppsListFilePath() const
{
    return QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + QLatin1String("/mimeapps.list");
}
