/*
 * Copyright © 2021 Reion Wong <reionwong@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef CURSORTHEME_H
#define CURSORTHEME_H

#include <QObject>
#include <QDir>
#include <QPixmap>

class CursorTheme : public QObject
{
    Q_OBJECT

public:
    explicit CursorTheme(const QDir &dir, QObject *parent = nullptr);

    QString name() const;
    QString path() const;
    QString id() const;
    QString inherits() const;
    QPixmap pixmap() const;

private:
    QString m_dirName;
    QString m_path;
    QString m_inherits;
    QString m_name;
    QString m_sample;
    mutable QPixmap m_pixmap;
    bool m_hidden;
};

#endif // CURSORTHEME_H
