// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pagetest.h"

TEST(NotificationsPage, ShowsTheEmptyCardWithoutApplications)
{
    PageFixture page("notifications");
    EXPECT_EQ(page.count("settings::SwitchRow"), 1);
    EXPECT_EQ(page.count("settings::RowComboBox"), 0);
    EXPECT_EQ(page.count("settings::Footnote"), 0);
}
