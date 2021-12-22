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

#ifndef ABOUT_H
#define ABOUT_H

#include <QObject>
#include <QString>
#include <QSysInfo>
#include <qqml.h>

class About : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isCutefishOS READ isCutefishOS CONSTANT)
    Q_PROPERTY(QString version READ version CONSTANT)
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

    bool isCutefishOS();

    QString version();

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

    Q_INVOKABLE void openUpdator();

private:
    qlonglong calculateTotalRam() const;
};

#endif // ABOUT_H
