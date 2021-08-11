/*
 * Copyright © 2021 Reion Wong <reionwong@gmail.com>
 * Copyright © 2006-2007 Fredrik Höglund <fredrik@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 or at your option version 3 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef CURSORTHEME_H
#define CURSORTHEME_H

#include <QObject>
#include <QHash>
#include <QDir>
#include <QPixmap>

struct _XcursorImage;
struct _XcursorImages;

typedef _XcursorImage XcursorImage;
typedef _XcursorImages XcursorImages;

class QImage;
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

    int defaultCursorSize() const;

    XcursorImage *xcLoadImage(const QString &name, int size) const;
    XcursorImages *xcLoadImages(const QString &name, int size) const;

    QImage loadImage(const QString &name, int size) const;
    QPixmap createIcon() const;
    QPixmap createIcon(int size) const;

    qulonglong loadCursor(const QString &name, int size) const;

    QString findAlternative(const QString &name) const;
    QImage autoCropImage(const QImage &image) const;

    static bool haveXfixes();

private:
    QString m_dirName;
    QString m_path;
    QString m_inherits;
    QList<int> m_sizes;

    QString m_name;
    QString m_sample;
    mutable QPixmap m_pixmap;
    bool m_hidden;

    static QHash<QString, QString> m_alternatives;
};

#endif // CURSORTHEME_H
