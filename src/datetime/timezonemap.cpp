/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#include "timezonemap.h"
#include "timedated_interface.h"

#include <QFile>
#include <QDebug>
#include <QDateTime>
#include <QLocale>
#include <QTimeZone>

#include <limits>
#include <math.h>
#include <locale.h>

// gnome-control-center: cc-timezone-map.c
static double radians(double degrees) {
    return (degrees / 360.0) * M_PI * 2;
}

static double convert_latitude_to_y(double latitude) {
    const double bottom_lat = -59;
    const double top_lat = 81;
    const double full_range = 4.6068250867599998;
    double top_per, y, top_offset, map_range;

    top_per = top_lat / 180.0;
    y = 1.25 * log(tan(M_PI_4 + 0.4 * radians(latitude)));
    top_offset = full_range * top_per;
    map_range = fabs(1.25 * log(tan(M_PI_4 + 0.4 * radians(bottom_lat))) - top_offset);
    y = fabs(y - top_offset);
    y = y / map_range;

    return y;
}

static double convert_longitude_to_x(double longitude) {
    const double xdeg_offset = -6;
    return ((180.0 + longitude) / 360.0 + xdeg_offset / 180.0);
}

// Parse latitude and longitude of the zone's principal location.
// See https://en.wikipedia.org/wiki/List_of_tz_database_time_zones
// |pos| is in ISO 6709 sign-degrees-minutes-seconds format,
// either +-DDMM+-DDDMM or +-DDMMSS+-DDDMMSS.
// |digits| 2 for latitude, 3 for longitude.
double convert_pos(const QString &pos, int digits) {
    if (pos.length() < 4 || digits > 9) {
        return 0.0;
    }

    const QString integer = pos.left(digits + 1);
    const QString fraction = pos.mid(digits + 1);
    const double t1 = integer.toDouble();
    const double t2 = fraction.toDouble();
    if (t1 > 0.0) {
        return t1 + t2 / pow(10.0, fraction.length());
    } else {
        return t1 - t2 / pow(10.0, fraction.length());
    }
}

TimeZoneMap::TimeZoneMap(QObject *parent)
    : QObject(parent)
{
    initDatas();
}

TimeZoneMap::~TimeZoneMap()
{
    qDeleteAll(m_list);
}

int TimeZoneMap::nearestIndex(int x, int y, int width, int height) const
{
    double minimum_distance = std::numeric_limits<double>::max();
    int nearest_zone_index = -1;

    for (int i = 0; i < m_list.size(); ++i) {
        const TimeZoneItem *item = m_list.at(i);
        const double dx = convert_longitude_to_x(item->longitude) * width - x;
        const double dy = convert_latitude_to_y(item->latitude) * height - y;
        const double distance = dx * dx + dy * dy;

        if (distance < minimum_distance) {
            minimum_distance = distance;
            nearest_zone_index = i;
        }
    }

    return nearest_zone_index;
}

void TimeZoneMap::clicked(int x, int y, int width, int height)
{
    QList<TimeZoneItem *> items;

    for (TimeZoneItem *item : m_list) {
        const double dx = convert_longitude_to_x(item->longitude) * width - x;
        const double dy = convert_latitude_to_y(item->latitude) * height - y;

        if (dx * dx + dy * dy <= 20) {
            items << item;
        }
    }

    if (items.isEmpty()) {
        const int nearest_zone_index = nearestIndex(x, y, width, height);
        if (nearest_zone_index != -1)
            items.append(m_list.at(nearest_zone_index));
    }

    m_currentList.clear();
    for (TimeZoneItem *item : items) {
        m_currentList.append(item->timeZone);
    }

    std::sort(m_currentList.begin(), m_currentList.end());

    emit availableListChanged();
}

void TimeZoneMap::setTimeZone(QString value)
{
    if (value.isEmpty())
        return;

    OrgFreedesktopTimedate1Interface iface(QStringLiteral("org.freedesktop.timedate1"),
                                                   QStringLiteral("/org/freedesktop/timedate1"),
                                                   QDBusConnection::systemBus());

    auto reply = iface.SetTimezone(value, true);
    reply.waitForFinished();

    if (reply.isError()) {
        qWarning() << "Failed to set timezone" << reply.error().message();
        return;
    }

    m_currentTimeZone = QTimeZone(value.toLatin1()).id();
    emit currentTimeZoneChanged();
}

void TimeZoneMap::select(const QString &timeZone)
{
    if (timeZone.isEmpty())
        return;
    m_currentList = QStringList {timeZone};
    emit availableListChanged();
}

QPointF TimeZoneMap::position(const QString &timeZone, int width, int height) const
{
    for (const TimeZoneItem *item : m_list) {
        if (item->timeZone == timeZone) {
            return QPointF(convert_longitude_to_x(item->longitude) * width,
                           convert_latitude_to_y(item->latitude) * height);
        }
    }
    return QPointF(-1, -1);
}

QString TimeZoneMap::nearest(int x, int y, int width, int height) const
{
    const int index = nearestIndex(x, y, width, height);
    return index < 0 ? QString() : m_list.at(index)->timeZone;
}

QVariantList TimeZoneMap::points() const
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    QVariantList result;
    result.reserve(m_list.size());
    for (const TimeZoneItem *item : m_list) {
        result.append(QVariantMap {
                {QStringLiteral("id"), item->timeZone},
                {QStringLiteral("x"), convert_longitude_to_x(item->longitude)},
                {QStringLiteral("y"), convert_latitude_to_y(item->latitude)},
                {QStringLiteral("offsetSeconds"), QTimeZone(item->timeZone.toLatin1()).offsetFromUtc(now)},
        });
    }
    return result;
}

QVariantMap TimeZoneMap::info(const QString &timeZone) const
{
    const TimeZoneItem *found = nullptr;
    for (const TimeZoneItem *item : m_list) {
        if (item->timeZone == timeZone) {
            found = item;
            break;
        }
    }
    if (!found)
        return {};

    // Offsets are not something most people think in: the gap to the zone
    // in use, and the day when it differs, say the same thing plainly.
    const QDateTime utc = QDateTime::currentDateTimeUtc();
    const QTimeZone zone(found->timeZone.toLatin1());
    const QTimeZone here(m_currentTimeZone.toLatin1());
    const QDateTime there = utc.toTimeZone(zone);
    const QDateTime local = utc.toTimeZone(here);
    const int offset = zone.offsetFromUtc(utc);
    const int gap = offset - here.offsetFromUtc(utc);

    QString difference;
    const int hours = qAbs(gap) / 3600;
    const int minutes = qAbs(gap) % 3600 / 60;
    if (gap == 0)
        difference = tr("Same time as now");
    // Singular and plural spelled out: the English text ships without a
    // translation, where "%n hour(s)" would show as written.
    else if (minutes == 0 && hours == 1)
        difference = gap > 0 ? tr("1 hour ahead") : tr("1 hour behind");
    else if (minutes == 0)
        difference = gap > 0 ? tr("%1 hours ahead").arg(hours) : tr("%1 hours behind").arg(hours);
    else if (hours == 0)
        difference = gap > 0 ? tr("%1 min ahead").arg(minutes) : tr("%1 min behind").arg(minutes);
    else
        difference = gap > 0 ? tr("%1 h %2 min ahead").arg(hours).arg(minutes)
                             : tr("%1 h %2 min behind").arg(hours).arg(minutes);

    QString day;
    const qint64 days = local.date().daysTo(there.date());
    if (days > 0)
        day = tr("Tomorrow");
    else if (days < 0)
        day = tr("Yesterday");

    return QVariantMap {
            {QStringLiteral("id"), found->timeZone},
            {QStringLiteral("name"), localeTimeZoneName(found->timeZone)},
            {QStringLiteral("region"),
             QLocale::territoryToString(QLocale::codeToTerritory(found->country))},
            {QStringLiteral("offsetSeconds"), offset},
            {QStringLiteral("time"), QLocale::system().toString(there.time(), QLocale::ShortFormat)},
            {QStringLiteral("day"), day},
            {QStringLiteral("difference"), difference},
    };
}

QVariantList TimeZoneMap::search(const QString &text, int limit) const
{
    const QString needle = text.trimmed();
    if (needle.isEmpty())
        return {};

    struct Match {
        int rank;
        const TimeZoneItem *item;
    };
    QList<Match> matches;
    for (const TimeZoneItem *item : m_list) {
        // "New_York" is typed as "new york"; the country code as "US".
        const QString id = QString(item->timeZone).replace(QLatin1Char('_'), QLatin1Char(' '));
        const QString city = localeTimeZoneName(id);
        const QString country = QLocale::territoryToString(
                QLocale::codeToTerritory(item->country));
        int rank = -1;
        if (city.startsWith(needle, Qt::CaseInsensitive))
            rank = 0;
        else if (id.startsWith(needle, Qt::CaseInsensitive))
            rank = 1;
        else if (city.contains(needle, Qt::CaseInsensitive)
                 || id.contains(needle, Qt::CaseInsensitive))
            rank = 2;
        else if (country.startsWith(needle, Qt::CaseInsensitive)
                 || item->country.compare(needle, Qt::CaseInsensitive) == 0)
            rank = 3;
        if (rank >= 0)
            matches.append({rank, item});
    }
    std::stable_sort(matches.begin(), matches.end(), [](const Match &a, const Match &b) {
        return a.rank != b.rank ? a.rank < b.rank : a.item->timeZone < b.item->timeZone;
    });

    QVariantList result;
    for (const Match &match : matches) {
        if (result.size() >= limit)
            break;
        result.append(info(match.item->timeZone));
    }
    return result;
}

QString TimeZoneMap::currentTimeZone() const
{
    return localeTimeZoneName(m_currentTimeZone);
}

QStringList TimeZoneMap::availableList()
{
    return m_currentList;
}

QString TimeZoneMap::localeTimeZoneName(const QString &timeZone) const
{
    const QString locale = QLocale::system().name();
    setlocale(LC_ALL, QString(locale + ".UTF-8").toStdString().c_str());
    int index = timeZone.lastIndexOf('/');
    setlocale(LC_ALL, "en_US.UTF-8");
    const QString city = (index > -1) ? timeZone.mid(index + 1) : timeZone;
    // "Los_Angeles" is written that way in the database only.
    return QString(city).replace(QLatin1Char('_'), QLatin1Char(' '));
}

void TimeZoneMap::initDatas()
{
    QFile zoneTab("/usr/share/zoneinfo/zone.tab");
    if (!zoneTab.open(QIODevice::ReadOnly)) {
        return;
    }

    const auto lines = QString::fromUtf8(zoneTab.readAll()).split(QLatin1Char('\n'));
    for (const QString &line : lines) {
        if (line.startsWith('#'))
            continue;

        const QStringList parts = line.split('\t');
        if (parts.size() < 3)
            continue;

        const QString coordinate = parts.at(1);
        int index = coordinate.indexOf('+', 3);
        if (index == -1) {
            index = coordinate.indexOf('-', 3);
        }

        const double latitude = convert_pos(coordinate.left(index), 2);
        const double longitude = convert_pos(coordinate.mid(index), 3);

        TimeZoneItem *item = new TimeZoneItem;
        item->country = parts.at(0);
        item->timeZone = parts.at(2);
        item->latitude = latitude;
        item->longitude = longitude;
        m_list.append(item);
    }
    std::sort(m_list.begin(), m_list.end());

    // currentTimeZoneChanged
    m_currentTimeZone = QTimeZone(QTimeZone::systemTimeZoneId()).id();
    emit currentTimeZoneChanged();
}
