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

Fonts::Fonts(QObject *parent)
    : QObject(parent)
    , m_settings("cutefishos", "theme")
    , m_antiAliasing(false)
    , m_hintingModel(new QStandardItemModel(this))
{
    m_antiAliasing = m_settings.value("XftAntialias", true).toBool();
    m_hinting = KXftConfig::toHintStyle(m_settings.value("XftHintStyle", "hintslight").toString());

    // Hinting options
    for (KXftConfig::Hint::Style s : {KXftConfig::Hint::None, KXftConfig::Hint::Slight, KXftConfig::Hint::Medium, KXftConfig::Hint::Full}) {
        auto item = new QStandardItem(KXftConfig::description(s));
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
    }
}

int Fonts::hintingCurrentIndex() const
{
    return hinting() - KXftConfig::Hint::None;
}

void Fonts::setHintingCurrentIndex(int index)
{
    setHinting(static_cast<KXftConfig::Hint::Style>(KXftConfig::Hint::None + index));
}

KXftConfig::Hint::Style Fonts::hinting() const
{
    return m_hinting;
}

void Fonts::setHinting(KXftConfig::Hint::Style hinting)
{
    if (m_hinting != hinting) {
        m_hinting = hinting;
        save();
        emit hintingChanged();
    }
}

QStandardItemModel *Fonts::hintingModel()
{
    return m_hintingModel;
}

void Fonts::save()
{
//    KXftConfig xft;
//    KXftConfig::AntiAliasing::State aaState = KXftConfig::AntiAliasing::NotSet;
//    if (xft.antiAliasingHasLocalConfig()) {
//        aaState = m_antiAliasing ? KXftConfig::AntiAliasing::Enabled : KXftConfig::AntiAliasing::Disabled;
//    }
//    xft.setAntiAliasing(aaState);
//    xft.setHintStyle(m_hinting);
//    xft.apply();

    m_settings.setValue("XftAntialias", m_antiAliasing);
    m_settings.setValue("XftHintStyle", KXftConfig::toStr(m_hinting));
    m_settings.sync();

    QDBusInterface interface("com.cutefish.Settings",
                             "/Theme",
                             "com.cutefish.Theme",
                             QDBusConnection::sessionBus());
    if (interface.isValid())
        interface.asyncCall("applyXResources");
}
