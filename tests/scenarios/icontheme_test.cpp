// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pages/icontheme/iconthemes.h"
#include "scenario.h"

TEST(IconThemePage, ListsTheInstalledThemes)
{
    const auto themes = settings::IconTheme::installed();
    Scenario scenario("appearance", "icontheme");
    EXPECT_EQ(scenario.driver.countShown("icontheme:theme"), themes.size());
}
