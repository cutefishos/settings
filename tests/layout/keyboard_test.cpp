// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pages/keyboard/keyboard.h"
#include "pagetest.h"
#include "ui/window.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using namespace settings;
using settings::testing::windowRect;

TEST(KeyboardLayoutPage, TheListFillsThePageBelowTheSearch)
{
    if (keyboardLayouts().empty())
        GTEST_SKIP() << "no xkb rules file";
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    settingsWindow.switchToPage("keyboard");
    settingsWindow.pushPage("keyboardlayout");
    window.grab();

    // The list fills the page below the search field, with the page margins.
    const RectF search = windowRect(window, "keyboardlayout:search");
    const RectF list = windowRect(window, "keyboardlayout:list");
    EXPECT_EQ(search.x, 254);
    EXPECT_EQ(search.y, 52);
    EXPECT_EQ(list.y, search.y + search.height + 16);
    EXPECT_EQ(list.y + list.height, 610 - 24);
}
