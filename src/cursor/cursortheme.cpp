/*
 * Copyright © 2021 Reion Wong <reionwong@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#include "cursortheme.h"

#include <QIcon>
#include <QSettings>
#include <X11/Xcursor/Xcursor.h>

CursorTheme::CursorTheme(const QDir &dir, QObject *parent)
    : QObject(parent)
    , m_dirName(dir.dirName())
    , m_path(dir.path())
    , m_sample(QStringLiteral("left_ptr"))
    , m_hidden(false)
{
    QSettings config(m_path
                             + (dir.exists("index.theme") ? QStringLiteral("/index.theme")
                                                          : QStringLiteral("/cursor.theme")),
                     QSettings::IniFormat);
    config.beginGroup(QStringLiteral("Icon Theme"));
    m_name = config.value(QStringLiteral("Name"), m_dirName).toString();
    m_sample = config.value(QStringLiteral("Example"), m_sample).toString();
    m_hidden = config.value(QStringLiteral("Hidden"), false).toBool();
    m_inherits = config.value(QStringLiteral("Inherits")).toString();
}

QString CursorTheme::name() const
{
    return m_name;
}

QString CursorTheme::path() const
{
    return m_path;
}

QString CursorTheme::id() const
{
    return m_dirName;
}

QString CursorTheme::inherits() const
{
    return m_inherits;
}

QPixmap CursorTheme::pixmap() const
{
    if (m_pixmap.isNull()) {
        XcursorImages *images = XcursorLibraryLoadImages(m_sample.toUtf8().constData(),
                                                         m_dirName.toUtf8().constData(), 36);
        if (!images)
            images = XcursorLibraryLoadImages("left_ptr", m_dirName.toUtf8().constData(), 36);
        if (images) {
            if (images->nimage > 0) {
                const auto *frame = images->images[0];
                QImage image(reinterpret_cast<const uchar *>(frame->pixels), frame->width,
                             frame->height, frame->width * 4, QImage::Format_ARGB32_Premultiplied);
                m_pixmap = QPixmap::fromImage(image.copy());
            }
            XcursorImagesDestroy(images);
        }
    }

    return m_pixmap;
}
