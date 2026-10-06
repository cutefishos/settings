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
    Q_PROPERTY(QVariantList videoPlayerList READ videoPlayerList NOTIFY loadFinished)
    Q_PROPERTY(int browserIndex READ browserIndex NOTIFY loadFinished)
    Q_PROPERTY(int emailIndex READ emailIndex NOTIFY loadFinished)
    Q_PROPERTY(int terminalIndex READ terminalIndex NOTIFY loadFinished)
    Q_PROPERTY(int videoPlayerIndex READ videoPlayerIndex NOTIFY loadFinished)

public:
    explicit DefaultApplications(QObject *parent = nullptr);

    void loadApps();

    QVariantList browserList();
    QVariantList emailList();
    QVariantList terminalList();
    QVariantList videoPlayerList();

    int browserIndex();
    int emailIndex();
    int terminalIndex();
    int videoPlayerIndex();

    Q_INVOKABLE void setDefaultBrowser(int index);
    Q_INVOKABLE void setDefaultEMail(int index);
    Q_INVOKABLE void setDefaultTerminal(int index);
    Q_INVOKABLE void setDefaultVideoPlayer(int index);

private:
    void setDefaultApp(const QString &mimeType, const QString &path);

signals:
    void loadFinished();

private:
    QList<AppItem> m_browserList;
    QList<AppItem> m_emailList;
    QList<AppItem> m_terminalList;
    QList<AppItem> m_videoPlayerList;

    int m_browserIndex = -1;
    int m_emailIndex = -1;
    int m_terminalIndex = -1;
    int m_videoPlayerIndex = -1;
};

#endif // DEFAULTAPPLICATIONS_H
