// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include "ui/painter.h"
#include "ui/cell_layout.h"

namespace settings {

// A rounded panel on a page in FishUI's second background colour: the
// RoundedItem's ColumnLayout, its rows its children. A list card holds
// ListRows edge to edge, without padding, so a row's press background
// reaches the card's edge and its corners follow the card's.
class Card : public cutefish::ui::ColumnLayout
{
public:
    enum class Style {
        Padded,
        List,
    };

    explicit Card(Style style = Style::Padded);

protected:
    void paint(cutefish::ui::Painter& painter) override;
};


// Kept for pages written before the toolkit had it; new code calls
// cutefish::ui::themeCorners().
inline cutefish::ui::Corners panelCorners()
{
    return cutefish::ui::themeCorners();
}

} // namespace settings
