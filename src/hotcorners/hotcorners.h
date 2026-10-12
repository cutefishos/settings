// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HOTCORNERS_H
#define HOTCORNERS_H

#include <QDBusInterface>
#include <QObject>

// The compositor's com.cutefish.HotCorners1: what each screen corner runs.
class HotCorners : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(QString topLeft READ topLeft NOTIFY changed)
    Q_PROPERTY(int topLeftModifiers READ topLeftModifiers NOTIFY changed)
    Q_PROPERTY(QString topRight READ topRight NOTIFY changed)
    Q_PROPERTY(int topRightModifiers READ topRightModifiers NOTIFY changed)
    Q_PROPERTY(QString bottomLeft READ bottomLeft NOTIFY changed)
    Q_PROPERTY(int bottomLeftModifiers READ bottomLeftModifiers NOTIFY changed)
    Q_PROPERTY(QString bottomRight READ bottomRight NOTIFY changed)
    Q_PROPERTY(int bottomRightModifiers READ bottomRightModifiers NOTIFY changed)

public:
    explicit HotCorners(QObject *parent = nullptr);

    bool available() const;
    QString topLeft() const { return stringProperty("topLeft"); }
    int topLeftModifiers() const { return intProperty("topLeftModifiers"); }
    QString topRight() const { return stringProperty("topRight"); }
    int topRightModifiers() const { return intProperty("topRightModifiers"); }
    QString bottomLeft() const { return stringProperty("bottomLeft"); }
    int bottomLeftModifiers() const { return intProperty("bottomLeftModifiers"); }
    QString bottomRight() const { return stringProperty("bottomRight"); }
    int bottomRightModifiers() const { return intProperty("bottomRightModifiers"); }

    // `corner` is top-left, top-right, bottom-left or bottom-right; an empty
    // action turns it off.
    Q_INVOKABLE void setCorner(const QString &corner, const QString &action, int modifiers);
    // The modifiers held now, as the compositor sees them.
    Q_INVOKABLE int heldModifiers() const;
    // "Option + Shift" and the like, in the session's shortcut convention.
    Q_INVOKABLE QString modifierText(int modifiers) const;

signals:
    void changed();

private:
    void refreshInterface();
    QString stringProperty(const char *name) const;
    int intProperty(const char *name) const;

    QDBusInterface *m_interface = nullptr;
    bool m_commandKeys = false;
};

#endif // HOTCORNERS_H
