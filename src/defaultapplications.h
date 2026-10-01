// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEFAULTAPPLICATIONS_H
#define DEFAULTAPPLICATIONS_H

#include <QObject>
#include <QVariantList>

class AppItem {
public:
    QString path;
    QString fileName;
    QString name;
    QString icon;
    QString mimeType;
    QString categories;
};

class DefaultApplications : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList browserList READ browserList NOTIFY loadFinished)
    Q_PROPERTY(QVariantList emailList READ emailList NOTIFY loadFinished)
    Q_PROPERTY(QVariantList terminalList READ terminalList NOTIFY loadFinished)
    Q_PROPERTY(int browserIndex READ browserIndex NOTIFY loadFinished)
    Q_PROPERTY(int emailIndex READ emailIndex NOTIFY loadFinished)
    Q_PROPERTY(int terminalIndex READ terminalIndex NOTIFY loadFinished)

public:
    explicit DefaultApplications(QObject *parent = nullptr);

    void loadApps();

    QVariantList browserList();
    QVariantList emailList();
    QVariantList terminalList();

    int browserIndex();
    int emailIndex();
    int terminalIndex();

    Q_INVOKABLE void setDefaultBrowser(int index);
    Q_INVOKABLE void setDefaultEMail(int index);
    Q_INVOKABLE void setDefaultTerminal(int index);

private:
    void setDefaultApp(const QString &mimeType, const QString &path);
    QString mimeAppsListFilePath() const;
    QString terminalsListFilePath() const;

signals:
    void loadFinished();

private:
    QList<AppItem> m_browserList;
    QList<AppItem> m_emailList;
    QList<AppItem> m_terminalList;

    int m_browserIndex = -1;
    int m_emailIndex = -1;
    int m_terminalIndex = -1;
};

#endif // DEFAULTAPPLICATIONS_H
