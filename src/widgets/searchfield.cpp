// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/searchfield.h"

#include "widgets/metrics.h"

using namespace cutefish::ui;

namespace settings {

static constexpr double kHeight = 28;

SearchField::SearchField()
    : cutefish::ui::SearchField("Search")
{
    setIconMetrics(metrics::s, metrics::xs, metrics::xs);
    setLayoutHints({.fillWidth = true});
}

SizeF SearchField::sizeHint() const
{
    return {cutefish::ui::SearchField::sizeHint().width, kHeight};
}

} // namespace settings
