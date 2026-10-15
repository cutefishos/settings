// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "ui/search_field.h"

namespace settings {

// Settings' search field: the toolkit's pill at 28 high, its glyphs 8 from
// the ends and 4 from the text.
class SearchField : public cutefish::ui::SearchField
{
public:
    SearchField();

    cutefish::ui::SizeF sizeHint() const override;
};

} // namespace settings
