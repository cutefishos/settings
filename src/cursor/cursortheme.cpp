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

CursorTheme::CursorTheme(const QDir &dir, QObject *parent)
    : QObject(parent)
    , m_dirName(dir.dirName())
    , m_path(dir.path())
    , m_sample(QStringLiteral("left_ptr"))
    , m_hidden(false)
{
    QSettings config(m_path + QStringLiteral("/index.theme"), QSettings::IniFormat);
    config.beginGroup(QStringLiteral("Icon Theme"));
    m_name = config.value(QStringLiteral("Name")).toString();
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
        const QIcon icon = QIcon::fromTheme(m_sample,
                                             QIcon::fromTheme(QStringLiteral("input-mouse")));
        m_pixmap = icon.pixmap(36, 36);
    }

    return m_pixmap;
}
