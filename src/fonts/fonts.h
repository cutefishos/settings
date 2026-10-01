// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef FONTS_H
#define FONTS_H

#include <QObject>
#include <QStandardItemModel>

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
    bool m_antiAliasing;
    QStandardItemModel *m_hintingModel;
    Hinting m_hinting;
};

#endif
