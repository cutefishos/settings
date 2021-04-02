#ifndef ABOUT_H
#define ABOUT_H

#include <QObject>
#include <QString>
#include <QSysInfo>
#include <qqml.h>

class About : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString osName READ osName CONSTANT)
    Q_PROPERTY(QString architecture READ architecture CONSTANT)
    Q_PROPERTY(QString kernelVersion READ kernelVersion CONSTANT)
    Q_PROPERTY(QString hostname READ hostname CONSTANT)
    Q_PROPERTY(QString userName READ userName CONSTANT)
    Q_PROPERTY(QString memorySize READ memorySize CONSTANT)
    Q_PROPERTY(QString prettyProductName READ prettyProductName CONSTANT)
    Q_PROPERTY(QString internalStorage READ internalStorage CONSTANT)
    Q_PROPERTY(QString cpuInfo READ cpuInfo CONSTANT)

public:
    explicit About(QObject *parent = nullptr);

    QString osName();
    QString architecture();
    QString kernelType();
    QString kernelVersion();
    QString hostname();
    QString userName();
    QString settingsVersion();
    QString memorySize();
    QString prettyProductName();
    QString internalStorage();
    QString cpuInfo();

private:
    qlonglong calculateTotalRam() const;
};

#endif // ABOUT_H
