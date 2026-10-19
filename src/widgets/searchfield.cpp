// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/searchfield.h"

#include "base/i18n.h"
#include "widgets/metrics.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {

static constexpr double kHeight = 28;

SearchField::SearchField()
    : cutefish::ui::SearchField(tr("Search"))
{
    setIconMetrics(metrics::s, metrics::xs, metrics::xs);
    setLayoutHints({.fillWidth = true});
}

SizeF SearchField::sizeHint() const
{
    return {cutefish::ui::SearchField::sizeHint().width, kHeight};
}

} // namespace settings
