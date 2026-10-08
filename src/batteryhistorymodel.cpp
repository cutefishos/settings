// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "batteryhistorymodel.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDBusArgument>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusMetaType> // qDBusRegisterMetaType
#include <QDBusPendingReply>

#include <algorithm>
#include <limits>
#include <memory>

const QDBusArgument &operator<<(QDBusArgument &argument, const HistoryReply &data)
{
    argument.beginStructure();
    argument << data.time << data.value << data.charging;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, HistoryReply &attrs)
{
   arg.beginStructure();
   arg >> attrs.time >> attrs.value >> attrs.charging;
   arg.endStructure();
   return arg;
}

BatteryHistoryModel::BatteryHistoryModel(QObject *parent)
    : QObject(parent)
{
    m_type = BatteryHistoryModel::ChargeType;
    m_duration = 120;

    qDBusRegisterMetaType<HistoryReply>();
    qDBusRegisterMetaType<QList<HistoryReply> >();
}

void BatteryHistoryModel::setDevice(const QString &device)
{
    if (device == m_device) {
        return;
    }

    m_device = device;
    Q_EMIT deviceChanged();

    load();
}

void BatteryHistoryModel::setDuration(uint duration)
{
    if (duration == m_duration) {
        return;
    }

    m_duration = duration;
    Q_EMIT durationChanged();

    load();
}

void BatteryHistoryModel::setType(BatteryHistoryModel::HistoryType type)
{
    if (m_type == type) {
        return;
    }

    m_type = type;
    Q_EMIT typeChanged();

    load();
}


void BatteryHistoryModel::classBegin()
{
}

void BatteryHistoryModel::componentComplete()
{
    m_isComplete = true;
    load();
}

QVariantList BatteryHistoryModel::asPoints() const
{
    QVariantList points;
    points.reserve(m_values.count());
    foreach (const HistoryReply &h, m_values) {
        points.append(QPointF(h.time, h.value));
    }

    return points;
}

QVariantList BatteryHistoryModel::states() const
{
    QVariantList states;
    states.reserve(m_values.count());
    for (const HistoryReply &h : m_values) {
        states.append(h.charging);
    }

    return states;
}

int BatteryHistoryModel::count() const
{
    return m_values.count();
}

int BatteryHistoryModel::lastDataPointTime() const
{
    if (m_values.isEmpty()) {
        return 0;
    }

    return m_values.last().time;
}

int BatteryHistoryModel::largestValue() const
{
    if (m_values.isEmpty()) {
        return 0;
    }

    int max = 0; // TODO std::max or something?
    for (auto it = m_values.constBegin(), end = m_values.constEnd(); it != end; ++it) {
        if ((*it).value > max) {
            max = (*it).value;
        }
    }
    return max;
}

void BatteryHistoryModel::refresh()
{
    load();
}

void BatteryHistoryModel::load()
{
    if (!m_isComplete || m_device.isEmpty()) {
        return;
    }

    if (QCoreApplication::arguments().contains(QStringLiteral("--mock-battery"))) {
        m_values.clear();
        const uint now = QDateTime::currentSecsSinceEpoch();
        // Each day drains on battery, charges, is held at 90% by the charge
        // limit and is unplugged again, ending where the next day starts.
        const QList<double> chargeValues { 72, 66, 59, 52, 45, 38, 32, 44, 57,
                                           69, 80, 88, 90, 90, 86, 81, 77, 72 };
        const QList<double> rateValues { 7.2, 6.8, 7.5, 6.9, 7.1, 6.4, 6.0, 11.8, 12.4,
                                         11.1, 9.6, 7.2, 4.1, 3.9, 6.8, 7.4, 6.9, 6.6 };
        const QList<uint> states { 2, 2, 2, 2, 2, 2, 2, 1, 1,
                                   1, 1, 1, 1, 5, 2, 2, 2, 2 };
        // Days drain a little deeper or shallower than one another.
        const QList<double> depths { 0, -6, 4, -3, 6, -4, 2 };
        const auto &values = m_type == RateType ? rateValues : chargeValues;
        const uint days = std::max(1u, m_duration / 86400);
        const uint interval = m_duration / days / values.size();

        for (uint day = 0; day < days; ++day) {
            for (int i = 0; i < values.size(); ++i) {
                HistoryReply reply;
                reply.time = now - m_duration + (day * values.size() + i) * interval;
                reply.value = values.at(i);
                reply.charging = states.at(i);

                if (m_type == ChargeType && i >= 3 && i <= 8) {
                    reply.value += depths.at((days - 1 - day) % depths.size());
                }

                m_values.append(reply);
            }
        }

        HistoryReply current = m_values.last();
        current.time = now;
        m_values.append(current);

        Q_EMIT dataChanged();
        return;
    }

    const auto historyCall = [this](uint timespan, uint resolution) {
        auto msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.UPower"),
                                                  m_device,
                                                  QStringLiteral("org.freedesktop.UPower.Device"),
                                                  QStringLiteral("GetHistory"));
        if (m_type == RateType) {
            msg << QLatin1String("rate");
        } else { // m_type must = ChargeType
            msg << QLatin1String("charge");
        }

        msg << timespan << resolution;
        return QDBusConnection::systemBus().asyncCall(msg);
    };

    const QDBusPendingReply<QList<HistoryReply>> window = historyCall(m_duration, 100);

    // UPower returns only the samples inside the timespan and records one only
    // when the value changes, so the value at the window's start is the newest
    // sample before it, found in the whole history.
    const QDBusPendingReply<QList<HistoryReply>> all = historyCall(std::numeric_limits<uint>::max(),
                                                                    std::numeric_limits<uint>::max());

    auto pending = std::make_shared<int>(2);

    const auto finished = [this, window, all, pending](QDBusPendingCallWatcher *watcher) {
        watcher->deleteLater();

        if (--*pending > 0) {
            return;
        }

        m_values.clear();

        if (window.isError()) {
            qWarning() << "Failed to get device history from UPower" << window.error().message();
            return;
        }

        const uint now = QDateTime::currentSecsSinceEpoch();
        const uint start = now > m_duration ? now - m_duration : 0;
        const QList<HistoryReply> history = window.value();

        // The sample in force at the start, unless UPower restarted after it:
        // what came in between is unknown.
        if (!all.isError()) {
            const QList<HistoryReply> everything = all.value();
            HistoryReply before;

            for (auto it = everything.crbegin(); it != everything.crend(); ++it) {
                if (it->time < before.time) {
                    continue;
                }

                if (it->time > start) {
                    if (it->value <= 0) {
                        before = HistoryReply();
                    }
                    break;
                }

                before = *it;
            }

            if (before.value > 0) {
                before.time = start;
                m_values.append(before);
            }
        }

        // UPower returns the newest sample first, in the order it recorded them.
        // A sample older than the one recorded before it was stamped while the
        // clock was wrong (before NTP set it after boot), so it is dropped.
        for (auto it = history.crbegin(); it != history.crend(); ++it) {
            // A zero value marks a UPower restart, not a reading.
            if (it->value <= 0 || it->time > now) {
                continue;
            }

            if (!m_values.isEmpty() && it->time < m_values.last().time) {
                continue;
            }

            // The curve needs distinct times; of two readings in a second, keep the later.
            if (!m_values.isEmpty() && it->time == m_values.last().time) {
                m_values.last() = *it;
                continue;
            }

            m_values.append(*it);
        }

        // UPower records a sample only when the value changes, so the last one
        // still holds and the plot runs on to the present.
        if (!m_values.isEmpty() && m_values.last().time < now) {
            HistoryReply current = m_values.last();
            current.time = now;
            m_values.append(current);
        }

        Q_EMIT dataChanged();
    };

    connect(new QDBusPendingCallWatcher(window, this), &QDBusPendingCallWatcher::finished, this, finished);
    connect(new QDBusPendingCallWatcher(all, this), &QDBusPendingCallWatcher::finished, this, finished);
}
