// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MOUSE_H
#define MOUSE_H

#include <QDBusInterface>
#include <QObject>

class Mouse : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool leftHanded READ leftHanded WRITE setLeftHanded NOTIFY leftHandedChanged)
    Q_PROPERTY(bool acceleration READ acceleration WRITE setAcceleration NOTIFY accelerationChanged)
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll NOTIFY
                       naturalScrollChanged)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration
                       NOTIFY pointerAccelerationChanged)

public:
    explicit Mouse(QObject *parent = nullptr);
    ~Mouse();

    bool available() const;

    bool leftHanded() const;
    void setLeftHanded(bool enabled);

    bool acceleration() const;
    void setAcceleration(bool enabled);

    bool naturalScroll() const;
    void setNaturalScroll(bool enabled);

    qreal pointerAcceleration() const;
    void setPointerAcceleration(qreal value);

signals:
    void availableChanged();
    void leftHandedChanged();
    void accelerationChanged();
    void naturalScrollChanged();
    void pointerAccelerationChanged();

private:
    void refreshInterface();
    void notifyChanged();
    void call(const QString &method, const QVariant &value);
    QDBusInterface *m_interface = nullptr;
};

#endif // MOUSE_H
