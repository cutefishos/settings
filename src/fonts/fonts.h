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

#ifndef FONTS_H
#define FONTS_H

#include <QObject>
#include <QStandardItemModel>
#include <QSettings>

#include <appearance.h>

class Fonts : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool antiAliasing READ antiAliasing WRITE setAntiAliasing NOTIFY antiAliasingChanged)
    Q_PROPERTY(QStandardItemModel * hintingModel READ hintingModel CONSTANT)
    Q_PROPERTY(int hintingCurrentIndex READ hintingCurrentIndex WRITE setHintingCurrentIndex NOTIFY hintingCurrentIndexChanged)

public:
    enum class Hinting {
        None,
        Slight,
        Medium,
        Full,
    };
    Q_ENUM(Hinting)

    explicit Fonts(QObject *parent = nullptr);

    bool antiAliasing() const;
    void setAntiAliasing(bool antiAliasing);

    int hintingCurrentIndex() const;
    void setHintingCurrentIndex(int index);

    Hinting hinting() const;
    void setHinting(Hinting hinting);

    QStandardItemModel *hintingModel();

    void save();

signals:
    void antiAliasingChanged();
    void hintingChanged();
    void hintingCurrentIndexChanged();

private:
    Appearance m_appearance;
    QSettings m_settings;
    bool m_antiAliasing;
    QStandardItemModel *m_hintingModel;
    Hinting m_hinting;
};

#endif
