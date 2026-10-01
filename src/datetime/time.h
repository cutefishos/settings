// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIME_H
#define TIME_H

#include <QObject>
#include <QTime>
#include <QDate>

class Time : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool useNtp READ useNtp WRITE setUseNtp NOTIFY useNtpChanged)
    Q_PROPERTY(bool canNTP READ canNTP CONSTANT)
    Q_PROPERTY(QTime currentTime READ currentTime WRITE setCurrentTime NOTIFY currentTimeChanged)
    Q_PROPERTY(QDate currentDate READ currentDate WRITE setCurrentDate NOTIFY currentDateChanged)
    Q_PROPERTY(bool twentyFour READ twentyFour WRITE setTwentyFour NOTIFY twentyFourChanged)

public:
    explicit Time(QObject *parent = nullptr);

    bool useNtp() const;
    bool canNTP() const;
    void setUseNtp(bool enabled);

    Q_INVOKABLE void save();

    QTime currentTime() const;
    void setCurrentTime(const QTime &currentTime);

    QDate currentDate() const;
    void setCurrentDate(const QDate &currentDate);

    bool twentyFour() const;
    void setTwentyFour(bool t);

signals:
    void useNtpChanged();
    void currentTimeChanged();
    void currentDateChanged();
    void twentyFourChanged();

private:
    bool m_canNTP;
    bool m_useNtp;
    bool m_twentyFour;
    QTime m_currentTime;
    QDate m_currentDate;
};

#endif // TIME_H
