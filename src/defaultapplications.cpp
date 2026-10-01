// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defaultapplications.h"
#include "applicationregistry.h"
#include "desktopentry.h"
#include "mimeapps.h"

#include <QDebug>
#include <QFileInfo>

DefaultApplications::DefaultApplications(QObject *parent)
    : QObject(parent)
{
    ApplicationRegistry *registry = ApplicationRegistry::instance();
    connect(registry, &ApplicationRegistry::applicationsChanged, this,
            &DefaultApplications::loadApps);
    if (!registry->loading())
        loadApps();
}

void DefaultApplications::loadApps()
{
    m_browserList.clear();
    m_emailList.clear();
    m_terminalList.clear();
    m_browserIndex = -1;
    m_emailIndex = -1;
    m_terminalIndex = -1;

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
        }
    }

    const QString defaultBrowser =
            MimeApps::associations(MimeApps::userListPath(), "Default Applications",
                                   "x-scheme-handler/http").value(0);
    const QString defaultEMail =
            MimeApps::associations(MimeApps::userListPath(), "Default Applications",
                                   "x-scheme-handler/mailto").value(0);
    const QString defaultTerminal = MimeApps::defaultTerminal(MimeApps::userTerminalsListPath());

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

void DefaultApplications::setDefaultApp(const QString &mimeType, const QString &path)
{
    if (!MimeApps::setDefault(MimeApps::userListPath(), mimeType, path)) {
        qWarning() << "Failed to save default application for" << mimeType;
        return;
    }
    loadApps();
}
