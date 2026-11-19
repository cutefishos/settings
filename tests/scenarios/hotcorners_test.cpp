// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "scenario.h"

// The tests run on a private bus, where no compositor answers.
TEST(HotCornersPage, ExplainsWhenTheCompositorIsMissing)
{
    Scenario scenario("hotcorners");
    cutefish::ui::TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("hotcorners:unavailable"));
    EXPECT_EQ(driver.countShown("hotcorners:corner"), 0u);
    EXPECT_FALSE(driver.shows("hotcorners:note"));
}
