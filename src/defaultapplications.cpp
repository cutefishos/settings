// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defaultapplications.h"
#include "applicationregistry.h"
#include "desktopentry.h"
#include "mimeapps.h"

#include <QDebug>
#include <QFileInfo>
#include <QMimeDatabase>

static bool playsVideo(const DesktopEntry *entry)
{
    if (!entry->categories().contains(QStringLiteral("AudioVideo")))
        return false;
    for (const QString &mimeType : entry->mimeTypes()) {
        if (mimeType.startsWith(QLatin1String("video/")))
            return true;
    }
    return false;
}

DefaultApplications::DefaultApplications(QObject *parent)
    : QObject(parent)
{
    ApplicationRegistry *registry = ApplicationRegistry::instance();
    connect(registry, &ApplicationRegistry::applicationsChanged, this,
            &DefaultApplications::loadApps);
    if (!registry->loading())
        loadApps();

    // Other programs, the file manager among them, choose defaults too.
    m_reloadTimer.setSingleShot(true);
    m_reloadTimer.setInterval(100);
    connect(&m_reloadTimer, &QTimer::timeout, this, &DefaultApplications::loadApps);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, &m_reloadTimer, qOverload<>(&QTimer::start));
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, &m_reloadTimer, qOverload<>(&QTimer::start));
    watchLists();
}

void DefaultApplications::watchLists()
{
    // Lists are saved by replacing the file, which drops its watch; the folder sees them come back.
    for (const QString &path : {MimeApps::userListPath(), MimeApps::userTerminalsListPath()}) {
        if (QFileInfo::exists(path) && !m_watcher.files().contains(path))
            m_watcher.addPath(path);
    }
    const QString folder = QFileInfo(MimeApps::userListPath()).absolutePath();
    if (QFileInfo::exists(folder) && !m_watcher.directories().contains(folder))
        m_watcher.addPath(folder);
}

void DefaultApplications::loadApps()
{
    m_browserList.clear();
    m_emailList.clear();
    m_terminalList.clear();
    m_videoPlayerList.clear();
    m_browserIndex = -1;
    m_emailIndex = -1;
    m_terminalIndex = -1;
    m_videoPlayerIndex = -1;
    watchLists();

    for (DesktopEntry *entry : ApplicationRegistry::instance()->applicationEntries()) {
        AppItem item;
        item.path = entry->path();
        item.name = entry->name();
        item.icon = entry->icon();
        item.mimeType = entry->mimeTypes().join(QLatin1Char(';'));
        item.categories = entry->categories().join(QLatin1Char(';'));
        item.fileName = QFileInfo(entry->path()).fileName();

        if (entry->categories().contains(QStringLiteral("WebBrowser"))
            && entry->mimeTypes().contains(QStringLiteral("x-scheme-handler/http"))) {
            m_browserList.append(item);
        } else if (entry->categories().contains(QStringLiteral("Email"))
                   && entry->mimeTypes().contains(QStringLiteral("x-scheme-handler/mailto"))) {
            m_emailList.append(item);
        } else if (entry->categories().contains(QStringLiteral("TerminalEmulator"))) {
            m_terminalList.append(item);
        } else if (playsVideo(entry)) {
            m_videoPlayerList.append(item);
        }
    }

    const QString defaultBrowser =
            MimeApps::associations(MimeApps::userListPath(), "Default Applications",
                                   "x-scheme-handler/http").value(0);
    const QString defaultEMail =
            MimeApps::associations(MimeApps::userListPath(), "Default Applications",
                                   "x-scheme-handler/mailto").value(0);
    const QString defaultTerminal = MimeApps::defaultTerminal(MimeApps::userTerminalsListPath());
    // Players differ in the video types they list; the commonest one stands for all.
    // Without a choice, this is what the file manager opens videos with.
    const QString defaultVideoPlayer = MimeApps::defaultApplication(
            MimeApps::listPaths(), QMimeDatabase().mimeTypeForName(QStringLiteral("video/mp4")));

    // Init indexes.
    for (int i = 0; i < m_browserList.size(); ++i) {
        if (defaultBrowser == m_browserList.at(i).fileName) {
            m_browserIndex = i;
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

    for (int i = 0; i < m_videoPlayerList.size(); ++i) {
        if (defaultVideoPlayer == m_videoPlayerList.at(i).path) {
            m_videoPlayerIndex = i;
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

QVariantList DefaultApplications::videoPlayerList()
{
    QVariantList list;

    for (const AppItem &item : m_videoPlayerList) {
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

int DefaultApplications::emailIndex()
{
    return m_emailIndex;
}

int DefaultApplications::terminalIndex()
{
    return m_terminalIndex;
}

int DefaultApplications::videoPlayerIndex()
{
    return m_videoPlayerIndex;
}

void DefaultApplications::setDefaultBrowser(int index)
{
    if (index < 0 || index >= m_browserList.size())
        return;

    const QString desktop = m_browserList.at(index).fileName;

    setDefaultApp("x-scheme-handler/http", desktop);
    setDefaultApp("x-scheme-handler/https", desktop);
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

    if (!MimeApps::setDefaultTerminal(MimeApps::userTerminalsListPath(),
                                      m_terminalList.at(index).fileName)) {
        qWarning() << "Failed to save the default terminal";
        return;
    }
    loadApps();
}

void DefaultApplications::setDefaultVideoPlayer(int index)
{
    if (index < 0 || index >= m_videoPlayerList.size())
        return;

    const AppItem &item = m_videoPlayerList.at(index);
    const QStringList mimeTypes = item.mimeType.split(QLatin1Char(';'), Qt::SkipEmptyParts);
    const QString desktop = item.fileName;

    for (const QString &mimeType : mimeTypes) {
        if (!mimeType.startsWith(QLatin1String("video/")))
            continue;
        if (!MimeApps::setDefault(MimeApps::userListPath(), mimeType, desktop))
            qWarning() << "Failed to save default application for" << mimeType;
    }
    loadApps();
}

void DefaultApplications::setDefaultApp(const QString &mimeType, const QString &path)
{
    if (!MimeApps::setDefault(MimeApps::userListPath(), mimeType, path)) {
        qWarning() << "Failed to save default application for" << mimeType;
        return;
    }
    loadApps();
}
