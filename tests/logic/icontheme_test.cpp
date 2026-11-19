// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>

#include "pages/icontheme/iconthemes.h"

TEST(IconThemes, LeaveOutTheFallback)
{
    const auto themes = settings::IconTheme::installed();
    // hicolor is a fallback, not a theme to choose.
    EXPECT_TRUE(std::none_of(themes.begin(), themes.end(), [](const auto& theme) { return theme.id == "hicolor"; }));
}
