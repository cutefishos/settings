#ifndef TOUCHPAD_H
#define TOUCHPAD_H

#include <QObject>
#include <QDBusInterface>
#include <QDBusPendingCall>

class Touchpad : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available CONSTANT)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool tapToClick READ tapToClick WRITE setTapToClick NOTIFY tapToClickChanged)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration NOTIFY pointerAccelerationChanged)

public:
    explicit Touchpad(QObject *parent = nullptr);

    bool available() const;
    bool enabled() const;
    void setEnabled(bool enabled);

    bool tapToClick() const;
    void setTapToClick(bool enabled);

    qreal pointerAcceleration() const;
    void setPointerAcceleration(qreal value);

signals:
    void enabledChanged();
    void tapToClickChanged();
    void pointerAccelerationChanged();

private:
    QDBusInterface m_iface;
};

#endif // TOUCHPAD_H
