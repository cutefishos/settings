// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cursortheme.h"

#include <QIcon>
#include <QPainter>
#include <QSettings>
#include <X11/Xcursor/Xcursor.h>

namespace {

QImage centeredCursorImage(const QImage &image)
{
    int left = image.width();
    int top = image.height();
    int right = -1;
    int bottom = -1;

    for (int y = 0; y < image.height(); ++y) {
        const auto *pixels = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(pixels[x]) == 0)
                continue;

            left = qMin(left, x);
            top = qMin(top, y);
            right = qMax(right, x);
            bottom = qMax(bottom, y);
        }
    }

    if (right < left || bottom < top)
        return image.copy();

    const QRect contentBounds(left, top, right - left + 1, bottom - top + 1);
    constexpr int padding = 2;
    const int extent = qMax(contentBounds.width(), contentBounds.height()) + padding * 2;

    QImage centered(extent, extent, QImage::Format_ARGB32_Premultiplied);
    centered.fill(Qt::transparent);

    QPainter painter(&centered);
    painter.drawImage((extent - contentBounds.width()) / 2,
                      (extent - contentBounds.height()) / 2, image, contentBounds.x(),
                      contentBounds.y(), contentBounds.width(), contentBounds.height());
    return centered;
}

}

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
                m_pixmap = QPixmap::fromImage(centeredCursorImage(image));
            }
            XcursorImagesDestroy(images);
        }
    }

    return m_pixmap;
}
