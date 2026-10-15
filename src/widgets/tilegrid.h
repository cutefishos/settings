// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/view.h"

namespace settings {

// Choices in cells of one size, as many to a row as fit, each at the cell's
// left and centred in its height, with some room below the last row.
class TileGrid : public cutefish::ui::View
{
public:
    TileGrid(cutefish::ui::SizeF cell, double bottomMargin = 0);

    cutefish::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void layout() override;

private:
    int columns(double width) const;
    int visibleCount() const;

    cutefish::ui::SizeF m_cell;
    double m_bottomMargin;
};

} // namespace settings
