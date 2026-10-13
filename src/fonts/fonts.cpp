// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "fonts.h"

#include <QSettings>

#include <preferences.h>

static Fonts::Hinting hintingFromString(const QString &value)
{
    if (value == QLatin1String("hintmedium"))
        return Fonts::Hinting::Medium;
    if (value == QLatin1String("hintfull"))
        return Fonts::Hinting::Full;
    if (value == QLatin1String("hintslight"))
        return Fonts::Hinting::Slight;
    return Fonts::Hinting::None;
}

static QString hintingToString(Fonts::Hinting hinting)
{
    switch (hinting) {
    case Fonts::Hinting::Slight:
        return QStringLiteral("hintslight");
    case Fonts::Hinting::Medium:
        return QStringLiteral("hintmedium");
    case Fonts::Hinting::Full:
        return QStringLiteral("hintfull");
    case Fonts::Hinting::None:
        return QStringLiteral("hintnone");
    }
    return QStringLiteral("hintnone");
}

Fonts::Fonts(QObject *parent)
    : QObject(parent)
    , m_antiAliasing(false)
    , m_hintingModel(new QStandardItemModel(this))
{
    QSettings settings(Preferences::file(QStringLiteral("services")), QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Appearance"));
    m_antiAliasing = settings.value("FontAntialias", true).toBool();
    m_hinting = hintingFromString(settings.value("FontHintStyle", "hintfull").toString());

    const QStringList descriptions = {tr("None"), tr("Slight"), tr("Medium"), tr("Full")};
    for (const QString &description : descriptions) {
        auto item = new QStandardItem(description);
        m_hintingModel->appendRow(item);
    }
}

bool Fonts::antiAliasing() const
{
    return m_antiAliasing;
}

void Fonts::setAntiAliasing(bool antiAliasing)
{
    if (m_antiAliasing != antiAliasing) {
        m_antiAliasing = antiAliasing;
        save();
        emit antiAliasingChanged();
    }
}

int Fonts::hintingCurrentIndex() const
{
    return static_cast<int>(hinting());
}

void Fonts::setHintingCurrentIndex(int index)
{
    index = qBound(0, index, 3);
    setHinting(static_cast<Hinting>(index));
}

Fonts::Hinting Fonts::hinting() const
{
    return m_hinting;
}

void Fonts::setHinting(Hinting hinting)
{
    if (m_hinting != hinting) {
        m_hinting = hinting;
        save();
        emit hintingChanged();
        emit hintingCurrentIndexChanged();
    }
}

QStandardItemModel *Fonts::hintingModel()
{
    return m_hintingModel;
}

void Fonts::save()
{
    m_appearance.setFontRendering(m_antiAliasing, hintingToString(m_hinting));
}
