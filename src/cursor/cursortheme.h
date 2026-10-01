// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CURSORTHEME_H
#define CURSORTHEME_H

#include <QDir>
#include <QObject>
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
