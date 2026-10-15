// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "pagetest.h"

TEST(DockPage, HasEveryControl)
{
    PageFixture page("dock");
    EXPECT_EQ(page.count("settings::ImageSelectionItem"), 3);
    EXPECT_EQ(page.count("settings::SliderRow"), 2);
    EXPECT_EQ(page.count("SegmentedControl"), 1);
    EXPECT_EQ(page.count("settings::NavigationItem"), 1);
    EXPECT_EQ(page.count("settings::SwitchRow"), 1);
}
