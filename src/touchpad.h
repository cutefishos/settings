/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
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

#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <QDBusInterface>
#include <QObject>

class Touchpad : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool disableWithMouse READ disableWithMouse WRITE setDisableWithMouse NOTIFY
                       disableWithMouseChanged)
    Q_PROPERTY(bool disableWhileTyping READ disableWhileTyping WRITE setDisableWhileTyping NOTIFY
                       disableWhileTypingChanged)
    Q_PROPERTY(bool tapToClick READ tapToClick WRITE setTapToClick NOTIFY tapToClickChanged)
    Q_PROPERTY(bool tapDragging READ tapDragging WRITE setTapDragging NOTIFY tapDraggingChanged)
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll NOTIFY
                       naturalScrollChanged)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration
                       NOTIFY pointerAccelerationChanged)

public:
    explicit Touchpad(QObject *parent = nullptr);

    bool available() const;

    bool enabled() const;
    void setEnabled(bool enabled);

    bool disableWithMouse() const;
    void setDisableWithMouse(bool enabled);

    bool disableWhileTyping() const;
    void setDisableWhileTyping(bool enabled);

    bool tapToClick() const;
    void setTapToClick(bool enabled);

    bool tapDragging() const;
    void setTapDragging(bool enabled);

    bool naturalScroll() const;
    void setNaturalScroll(bool enabled);

    qreal pointerAcceleration() const;
    void setPointerAcceleration(qreal value);

signals:
    void availableChanged();
    void enabledChanged();
    void disableWithMouseChanged();
    void disableWhileTypingChanged();
    void tapToClickChanged();
    void tapDraggingChanged();
    void naturalScrollChanged();
    void pointerAccelerationChanged();

private:
    void refreshInterface();
    void notifyChanged();
    void call(const QString &method, const QVariant &value);
    bool flag(const char *name, bool fallback) const;
    QDBusInterface *m_interface = nullptr;
};

#endif // TOUCHPAD_H
