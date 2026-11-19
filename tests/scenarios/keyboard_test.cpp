// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pages/keyboard/keyboard.h"
#include "scenario.h"

using namespace cutefish::ui;
using namespace settings;

TEST(KeyboardLayoutPage, FiltersByNameOrIdAndSaysWhenNothingMatches)
{
    if (keyboardLayouts().empty())
        GTEST_SKIP() << "no xkb rules file";
    Scenario scenario("keyboard", "keyboardlayout");
    TestDriver& driver = scenario.driver;
    const View* empty = driver.find("keyboardlayout:empty");
    ASSERT_NE(empty, nullptr);
    EXPECT_FALSE(empty->isVisible());

    driver.setKeyboardFocus(true);
    driver.click("keyboardlayout:search");
    driver.typeText("zzzz");
    EXPECT_TRUE(driver.shows("keyboardlayout:empty"));
}
