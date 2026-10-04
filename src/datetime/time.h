// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIME_H
#define TIME_H

#include <QObject>

class Time : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool useNtp READ useNtp WRITE setUseNtp NOTIFY useNtpChanged)
    Q_PROPERTY(bool canNTP READ canNTP CONSTANT)
    Q_PROPERTY(bool twentyFour READ twentyFour WRITE setTwentyFour NOTIFY twentyFourChanged)

public:
    explicit Time(QObject *parent = nullptr);

    bool useNtp() const;
    bool canNTP() const;
    void setUseNtp(bool enabled);

    bool twentyFour() const;
    void setTwentyFour(bool t);

signals:
    void useNtpChanged();
    void twentyFourChanged();

private:
    bool m_canNTP;
    bool m_useNtp;
    bool m_twentyFour;
};

#endif // TIME_H
