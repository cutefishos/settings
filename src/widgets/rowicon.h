// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <string>

#include "ui/view.h"

namespace cutefish::ui {
class Icon;
}

namespace settings {

class Picture;

// The sidebar's tile: a coloured rounded square with a white glyph, for the
// sidebar and a list row's leading slot. The glyph is one of settings'
// sidebar pictures, or a toolkit glyph when the name has no ".svg".
class RowIcon : public cutefish::ui::View
{
public:
    RowIcon(std::string glyph, cutefish::ui::Color tint);

    void set(std::string glyph, cutefish::ui::Color tint);

    cutefish::ui::SizeF sizeHint() const override { return {24, 24}; }

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;

private:
    cutefish::ui::Color m_tint;
    Picture* m_picture;
    cutefish::ui::Icon* m_glyph;
};

} // namespace settings
