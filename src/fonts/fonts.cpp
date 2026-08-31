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

#include "fonts.h"
#include <QDBusInterface>
#include <QDBusPendingCall>

namespace
{
Fonts::Hinting hintingFromString(const QString &value)
{
    if (value == QLatin1String("hintmedium"))
        return Fonts::Hinting::Medium;
    if (value == QLatin1String("hintfull"))
        return Fonts::Hinting::Full;
    if (value == QLatin1String("hintslight"))
        return Fonts::Hinting::Slight;
    return Fonts::Hinting::None;
}

QString hintingToString(Fonts::Hinting hinting)
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
}

Fonts::Fonts(QObject *parent)
    : QObject(parent)
    , m_settings("cutefishos", "theme")
    , m_antiAliasing(false)
    , m_hintingModel(new QStandardItemModel(this))
{
    m_antiAliasing = m_settings.value("FontAntialias", true).toBool();
    m_hinting = hintingFromString(m_settings.value("FontHintStyle", "hintslight").toString());

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
    m_settings.setValue("FontAntialias", m_antiAliasing);
    m_settings.setValue("FontHintStyle", hintingToString(m_hinting));
    m_settings.sync();

    QDBusInterface interface("com.cutefish.Settings",
                             "/Theme",
                             "com.cutefish.Theme",
                             QDBusConnection::sessionBus());
    if (interface.isValid())
        interface.asyncCall("applyFontSettings");
}
