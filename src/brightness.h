// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BRIGHTNESS_H
#define BRIGHTNESS_H

#include <QDBusInterface>
#include <QObject>

// Brightness of one display, addressed by its DRM connector name ("card0-eDP-1").
class Brightness : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString display READ display WRITE setDisplay NOTIFY displayChanged)
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(int value READ value NOTIFY valueChanged)

public:
    explicit Brightness(QObject *parent = nullptr);

    QString display() const;
    void setDisplay(const QString &display);

    bool available() const;
    int value() const;

    Q_INVOKABLE void setValue(int value);

signals:
    void displayChanged();
    void availableChanged();
    void valueChanged();

private slots:
    void refresh();
    void handleValueChanged(const QString &display, int value);

private:
    QDBusInterface m_iface;
    QString m_display;
    bool m_available = false;
    int m_value = 0;
};

#endif // BRIGHTNESS_H
