// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/rowicon.h"

#include "ui/icon.h"
#include "ui/painter.h"
#include "widgets/picture.h"

using namespace cutefish::ui;

namespace settings {

static constexpr double kGlyph = 16;

RowIcon::RowIcon(std::string glyph, Color tint)
{
    m_picture = addChild<Picture>();
    m_glyph = addChild<Icon>(std::string(), kGlyph);
    set(std::move(glyph), tint);
}

void RowIcon::set(std::string glyph, Color tint)
{
    m_tint = tint;
    const bool picture = glyph.ends_with(".svg");
    m_picture->setVisible(picture);
    m_glyph->setVisible(!picture);
    // The sidebar's own glyphs are white already; the toolkit's need tinting.
    if (picture) {
        m_picture->setPath(dataPath("images/sidebar/dark/" + glyph));
        m_picture->setTint(Color::rgb(0xFFFFFF));
    } else {
        m_glyph->setIconName(std::move(glyph));
        m_glyph->setColor(Color::rgb(0xFFFFFF));
    }
    requestPaint();
}

void RowIcon::layout()
{
    const RectF glyph{(geometry().width - kGlyph) / 2, (geometry().height - kGlyph) / 2, kGlyph, kGlyph};
    m_picture->setGeometry(glyph);
    m_glyph->setGeometry(glyph);
}

void RowIcon::paint(Painter& painter)
{
    painter.fillRoundedRect(localRect(), 6, LinearGradient::vertical(localRect(), m_tint.lighter(1.15), m_tint));
}

} // namespace settings
