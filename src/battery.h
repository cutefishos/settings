#ifndef BATTERY_H
#define BATTERY_H

#include <QObject>
#include <QDBusInterface>

class Battery : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY validChanged)
    Q_PROPERTY(int chargeState READ chargeState NOTIFY chargeStateChanged)
    Q_PROPERTY(int chargePercent READ chargePercent NOTIFY chargePercentChanged)
    Q_PROPERTY(int lastChargedPercent READ lastChargedPercent NOTIFY lastChargedPercentChanged)
    Q_PROPERTY(QString lastChargedTime READ lastChargedTime NOTIFY lastChargedPercentChanged)
    Q_PROPERTY(int capacity READ capacity NOTIFY capacityChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY remainingTimeChanged)
    Q_PROPERTY(bool onBattery READ onBattery NOTIFY onBatteryChanged)
    Q_PROPERTY(QString udi READ udi NOTIFY udiChanged)

public:
    explicit Battery(QObject *parent = nullptr);

    bool available() const;
    bool onBattery() const;

    Q_INVOKABLE void refresh();
    Q_INVOKABLE QVariantList getHistory(const int timespan, const int resolution);

    int chargeState() const;
    int chargePercent() const;
    int lastChargedPercent() const;
    int capacity() const;
    QString statusString() const;
    QString lastChargedTime() const;

    QString udi() const;

signals:
    void validChanged();
    void chargeStateChanged(int);
    void chargePercentChanged(int);
    void capacityChanged(int);
    void remainingTimeChanged(qlonglong time);
    void onBatteryChanged();
    void lastChargedPercentChanged();
    void udiChanged();

private slots:
    void onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);

private:
    QDBusInterface m_upowerInterface;
    QDBusInterface m_interface;
    bool m_available;
    bool m_onBattery;
    QString m_udi;
};

#endif // BATTERY_H
