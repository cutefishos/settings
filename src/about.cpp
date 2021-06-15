#include "about.h"

#include <QFile>
#include <QStorageInfo>
#include <QRegularExpression>

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
        return QObject::tr("%1 B").arg(numString);
    case 1:
        return QObject::tr("%1 KB").arg(numString);
    case 2:
        return QObject::tr("%1 MB").arg(numString);
    case 3:
        return QObject::tr("%1 GB").arg(numString);
    case 4:
        return QObject::tr("%1 TB").arg(numString);
    case 5:
        return QObject::tr("%1 PB").arg(numString);
    case 6:
        return QObject::tr("%1 EB").arg(numString);
    case 7:
        return QObject::tr("%1 ZB").arg(numString);
    case 8:
        return QObject::tr("%1 YB").arg(numString);
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
    return QFile::exists("/etc/cutefishos");
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

        int count = lines.filter(QRegularExpression("^processor")).count();

        QString result;
        result.append(modelLine.first().split(':').at(1));

        if (count > 0)
            result.append(QString(" x %1").arg(count));

        return result;
    }

    return QString();
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
