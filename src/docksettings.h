// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCKSETTINGS_H
#define DOCKSETTINGS_H

#include <QObject>

class DockSettings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int dockIconSize READ dockIconSize WRITE setDockIconSize NOTIFY dockIconSizeChanged)
    Q_PROPERTY(
            int dockDirection READ dockDirection WRITE setDockDirection NOTIFY dockDirectionChanged)
    Q_PROPERTY(int dockVisibility READ dockVisibility WRITE setDockVisibility NOTIFY
                       dockVisibilityChanged)
    Q_PROPERTY(qreal dockGlassTint READ dockGlassTint WRITE setDockGlassTint NOTIFY
                       dockGlassTintChanged)
    Q_PROPERTY(bool minimizedArea READ minimizedArea WRITE setMinimizedArea NOTIFY
                       minimizedAreaChanged)

public:
    explicit DockSettings(QObject *parent = nullptr);

    int dockIconSize() const;
    Q_INVOKABLE void setDockIconSize(int dockIconSize);

    int dockDirection() const;
    Q_INVOKABLE void setDockDirection(int dockDirection);

    int dockVisibility() const;
    Q_INVOKABLE void setDockVisibility(int visibility);

    qreal dockGlassTint() const;
    Q_INVOKABLE void setDockGlassTint(qreal tint);

    bool minimizedArea() const;
    Q_INVOKABLE void setMinimizedArea(bool enabled);

signals:
    void dockIconSizeChanged();
    void dockDirectionChanged();
    void dockVisibilityChanged();
    void dockGlassTintChanged();
    void minimizedAreaChanged();

private:
    int m_dockIconSize;
    int m_dockDirection;
    int m_dockVisibility;
    qreal m_dockGlassTint;
    bool m_minimizedArea;
};

#endif // DOCKSETTINGS_H
