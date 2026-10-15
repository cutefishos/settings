// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/card.h"

#include "ui/painter.h"
#include "ui/theme.h"
#include "widgets/metrics.h"

using namespace cutefish::ui;

namespace settings {

Card::Card(Style style)
{
    if (style == Style::Padded) {
        setPadding({metrics::cardHorizontalPadding, metrics::cardVerticalPadding, metrics::cardHorizontalPadding,
                    metrics::cardVerticalPadding});
        setSpacing(metrics::cardSpacing);
    } else {
        setSpacing(0);
    }
    setLayoutHints({.fillWidth = true});
}

void Card::paint(Painter& painter)
{
    const Theme& theme = Theme::current();
    painter.drawRoundedRect(localRect(), {theme.bigRadius, 0, themeCorners()}, theme.secondBackground);
}


} // namespace settings
