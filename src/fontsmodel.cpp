/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#include "fontsmodel.h"
#include <QFontDatabase>
#include <QFontInfo>
#include <QTimer>
#include <QDir>
#include <QDirIterator>

// Freetype and Fontconfig
#include <ft2build.h>
#include <freetype/freetype.h>
#include <fontconfig/fontconfig.h>
#include <ft2build.h>

FontsModel::FontsModel(QObject *parent)
    : QThread(parent)
{
    QThread::start();
    QThread::setPriority(QThread::HighestPriority);
}

void FontsModel::run()
{
    QString dirPath = "/usr/share/fonts";
    QDir dir(dirPath);

    if (!dir.exists()) {
        return;
    }

    m_generalFonts.clear();
    m_fixedFonts.clear();

    QStringList nameFilters;
    nameFilters << QLatin1String("*.ttf")
                << QLatin1String("*.ttc")
                << QLatin1String("*.pfa")
                << QLatin1String("*.pfb")
                << QLatin1String("*.otf");

    QDirIterator it(dirPath, nameFilters, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString filePath = it.next();

        // Init
        FT_Library library = nullptr;
        FT_Init_FreeType(&library);

        FT_Face face = nullptr;
        FT_Error error = FT_New_Face(library, filePath.toUtf8().constData(), 0, &face);

        if (error != FT_Err_Ok) {
            FT_Done_Face(face);
            FT_Done_FreeType(library);
            continue;
        }

        QString family = QString::fromLatin1(face->family_name);
        bool fixedPitch = (face->face_flags & FT_FACE_FLAG_FIXED_WIDTH);

        if (fixedPitch) {
            if (!m_fixedFonts.contains(family))
                m_fixedFonts.append(family);
        } else {
            if (!m_generalFonts.contains(family))
                m_generalFonts.append(family);
        }

        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    std::sort(m_fixedFonts.begin(), m_fixedFonts.end());
    std::sort(m_generalFonts.begin(), m_generalFonts.end());

    emit loadFinished();
}

QStringList FontsModel::generalFonts() const
{
    return m_generalFonts;
}

QStringList FontsModel::fixedFonts() const
{
    return m_fixedFonts;
}

QString FontsModel::systemGeneralFont() const
{
    return QFontDatabase::systemFont(QFontDatabase::GeneralFont).family();
}

QString FontsModel::systemFixedFont() const
{
    return QFontDatabase::systemFont(QFontDatabase::FixedFont).family();
}
