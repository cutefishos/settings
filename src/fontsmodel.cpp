// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fontsmodel.h"

#include <QDir>
#include <QDirIterator>
#include <QFontDatabase>
#include <QFontInfo>
#include <QLocale>
#include <QTimer>

// Freetype and Fontconfig
#include <fontconfig/fontconfig.h>
#include <freetype/freetype.h>
#include <ft2build.h>

static QString getFcString(FcPattern *pat, const char *val, int index)
{
    QString rv;
    FcChar8 *fcStr;

    if (FcPatternGetString(pat, val, index, &fcStr) == FcResultMatch)
        rv = QString::fromUtf8((char *)fcStr);

    return rv;
}

static QString getFcLangString(FcPattern *pat, const char *val, const char *valLang)
{
    QString rv;
    int langIndex = -1;

    for (int i = 0; true; ++i) {
        QString lang = getFcString(pat, valLang, i);

        if (lang.isEmpty())
            break;
        else if (QString::fromLatin1("en") == lang)
            return getFcString(pat, val, i);
        else if (QString::fromLatin1("xx") != lang && -1 == langIndex)
            langIndex = i;
    }

    return getFcString(pat, val, langIndex > 0 ? langIndex : 0);
}

FontsModel::FontsModel(QObject *parent)
    : QThread(parent)
{
    m_lanCode = QLocale::system().name().replace('_', "-");

    QThread::start();
    QThread::setPriority(QThread::HighestPriority);
}

// The page owning the model can be closed before the font scan finishes.
FontsModel::~FontsModel()
{
    wait();
}

void FontsModel::run()
{
    FcPattern *pat = FcPatternCreate();
    if (!pat)
        return;

    FcObjectSet *objSet = FcObjectSetBuild(FC_FAMILY, FC_FAMILYLANG, FC_CHARSET, FC_STYLE, FC_LANG,
                                           FC_SPACING, NULL);

    if (!objSet) {
        FcPatternDestroy(pat);
        return;
    }

    FcFontSet *fs = FcFontList(0, pat, objSet);
    FcObjectSetDestroy(objSet);
    FcPatternDestroy(pat);

    if (!fs) {
        return;
    }

    QStringList general;
    QStringList fixed;

    const QByteArray lanCode = m_lanCode.toUtf8();

    for (int i = 0; i < fs->nfont; ++i) {
        FcCharSet *charset = nullptr;
        if (FcPatternGetCharSet(fs->fonts[i], FC_CHARSET, 0, &charset) != FcResultMatch
            || FcCharSetCount(charset) == 0)
            continue;

        QString family = getFcLangString(fs->fonts[i], FC_FAMILY, FC_FAMILYLANG);
        if (family.isEmpty())
            continue;

        // Not FcPatternFormat("%{lang}"): Chrome's bundled fontconfig links its newer
        // caches as ours, and printing their lang sets indexes past our language table.
        FcLangSet *langSet = nullptr;
        bool supportsLanguage = FcPatternGetLangSet(fs->fonts[i], FC_LANG, 0, &langSet) == FcResultMatch
            && FcLangSetHasLang(langSet, (const FcChar8 *)lanCode.constData()) != FcLangDifferentLang;

        int spacing = FC_PROPORTIONAL;
        if (FcPatternGetInteger(fs->fonts[i], FC_SPACING, 0, &spacing) != FcResultMatch)
            spacing = FC_PROPORTIONAL;
        bool fixedPitch = spacing >= FC_MONO;

        if (fixedPitch && !fixed.contains(family)) {
            fixed.append(family);
        } else {
            if (!general.contains(family) && supportsLanguage)
                general.append(family);
        }
    }
    FcFontSetDestroy(fs);

    std::sort(fixed.begin(), fixed.end());
    std::sort(general.begin(), general.end());

    // QML reads the lists on the GUI thread, so they are handed over there. The
    // pending call is dropped along with the object if the page closes first.
    QMetaObject::invokeMethod(this,
            [this, general = std::move(general), fixed = std::move(fixed)] {
                m_generalFonts = general;
                m_fixedFonts = fixed;
                emit loadFinished();
            },
            Qt::QueuedConnection);
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
