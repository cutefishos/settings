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

#include "about.h"

#include <QFile>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QSettings>
#include <QProcess>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#elif defined(Q_OS_FREEBSD)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

static QString formatByteSize(double size, int precision)
{
    int unit = 0;
    double multiplier = 1024.0;

    while (qAbs(size) >= multiplier && unit < int(8)) {
        size /= multiplier;
        ++unit;
    }

    if (unit == 0) {
        precision = 0;
    }

    QString numString = QString::number(size, 'f', precision);

    switch (unit) {
    case 0:
        return QString("%1 B").arg(numString);
    case 1:
        return QString("%1 KB").arg(numString);
    case 2:
        return QString("%1 MB").arg(numString);
    case 3:
        return QString("%1 GB").arg(numString);
    case 4:
        return QString("%1 TB").arg(numString);
    case 5:
        return QString("%1 PB").arg(numString);
    case 6:
        return QString("%1 EB").arg(numString);
    case 7:
        return QString("%1 ZB").arg(numString);
    case 8:
        return QString("%1 YB").arg(numString);
    default:
        return QString();
    }

    return QString();
}

About::About(QObject *parent)
    : QObject(parent)
{
}

bool About::isCutefishOS()
{
    if (!QFile::exists("/etc/cutefishos"))
        return false;

    QSettings settings("/etc/cutefishos", QSettings::IniFormat);
    return settings.value("CutefishOS", false).toBool();
}

QString About::version()
{
    QSettings settings("/etc/cutefish", QSettings::IniFormat);
    return settings.value("Version").toString();
}

QString About::osName()
{
    return QSysInfo::prettyProductName();
}

QString About::architecture()
{
    return QSysInfo::currentCpuArchitecture();
}

QString About::kernelType()
{
    return QSysInfo::kernelType();
}

QString About::kernelVersion()
{
    return QSysInfo::kernelVersion();
}

QString About::hostname()
{
    return QSysInfo::machineHostName();
}

QString About::userName()
{
    QByteArray userName = qgetenv("USER");

    if (userName.isEmpty())
        userName = qgetenv("USERNAME");

    return QString::fromUtf8(userName);
}

QString About::memorySize()
{
    QString ram;
    const qlonglong totalRam = calculateTotalRam();

    if (totalRam > 0) {
        ram = formatByteSize(totalRam, 0);
    }
    return ram;
}

QString About::prettyProductName()
{
    return QSysInfo::prettyProductName();
}

QString About::internalStorage()
{
    QStorageInfo storage = QStorageInfo::root();
    return QString("%1 / %2")
            .arg(formatByteSize(storage.bytesTotal() - storage.bytesAvailable(), 0))
            .arg(formatByteSize(storage.bytesTotal(), 0));
}

QString About::cpuInfo()
{
    QFile file("/proc/cpuinfo");

    if (file.open(QIODevice::ReadOnly)) {
        QString buffer = file.readAll();
        QStringList modelLine = buffer.split('\n').filter(QRegularExpression("^model name"));
        QStringList lines = buffer.split('\n');

        if (modelLine.isEmpty())
            return "Unknown";

        int count = lines.filter(QRegularExpression("^processor")).count();

        QString result;
        result.append(modelLine.first().split(':').at(1));

        if (count > 0)
            result.append(QString(" x %1").arg(count));

        return result;
    }

    return QString();
}

void About::openUpdator()
{
    QProcess::startDetached("cutefish-updator", QStringList());
}

qlonglong About::calculateTotalRam() const
{
    qlonglong ret = -1;
#ifdef Q_OS_LINUX
    struct sysinfo info;
    if (sysinfo(&info) == 0)
        // manpage "sizes are given as multiples of mem_unit bytes"
        ret = qlonglong(info.totalram) * info.mem_unit;
#elif defined(Q_OS_FREEBSD)
    /* Stuff for sysctl */
    size_t len;

    unsigned long memory;
    len = sizeof(memory);
    sysctlbyname("hw.physmem", &memory, &len, NULL, 0);

    ret = memory;
#endif
    return ret;
}
