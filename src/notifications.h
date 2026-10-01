// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QVariantList>

class Notifications : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool doNotDisturb READ doNotDisturb WRITE setDoNotDisturb NOTIFY doNotDisturbChanged)
    Q_PROPERTY(QVariantList applications READ applications NOTIFY applicationsChanged)

public:
    // Matches NotificationSettings::ApplicationMode in the shell.
    enum ApplicationMode {
        Banners = 0,
        CenterOnly = 1,
        Off = 2,
    };
    Q_ENUM(ApplicationMode)

    explicit Notifications(QObject *parent = nullptr);

    bool doNotDisturb() const;
    void setDoNotDisturb(bool enabled);

    QVariantList applications() const;
    Q_INVOKABLE void setApplicationMode(const QString &key, int mode);

private slots:
    void onDBusDoNotDisturbChanged();
    void loadApplications();

signals:
    void doNotDisturbChanged();
    void applicationsChanged();

private:
    QDBusInterface m_iface;
    bool m_doNotDisturb;
    QVariantList m_applications;
};

#endif // NOTIFICATIONS_H
