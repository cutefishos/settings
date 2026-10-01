// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

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
    Q_PROPERTY(qreal scrollSpeed READ scrollSpeed WRITE setScrollSpeed NOTIFY scrollSpeedChanged)
    Q_PROPERTY(bool secondaryClickConfigurable READ secondaryClickConfigurable NOTIFY
                       secondaryClickConfigurableChanged)
    Q_PROPERTY(bool twoFingerSecondaryClick READ twoFingerSecondaryClick WRITE
                       setTwoFingerSecondaryClick NOTIFY twoFingerSecondaryClickChanged)

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

    qreal scrollSpeed() const;
    void setScrollSpeed(qreal value);

    bool secondaryClickConfigurable() const;

    bool twoFingerSecondaryClick() const;
    void setTwoFingerSecondaryClick(bool enabled);

signals:
    void availableChanged();
    void enabledChanged();
    void disableWithMouseChanged();
    void disableWhileTypingChanged();
    void tapToClickChanged();
    void tapDraggingChanged();
    void naturalScrollChanged();
    void pointerAccelerationChanged();
    void scrollSpeedChanged();
    void secondaryClickConfigurableChanged();
    void twoFingerSecondaryClickChanged();

private:
    void refreshInterface();
    void notifyChanged();
    void call(const QString &method, const QVariant &value);
    bool flag(const char *name, bool fallback) const;
    QDBusInterface *m_interface = nullptr;
};

#endif // TOUCHPAD_H
