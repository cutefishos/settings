// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <memory>

#include "pagetest.h"
#include "ui/layout.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using namespace settings;
using settings::testing::isHidden;
using settings::testing::windowRect;

TEST(Foundation, SidebarLinesUpWithTheMetrics)
{
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    window.grab();
    const RectF search = windowRect(window, "sidebar:search");
    EXPECT_EQ(search.x, 12);
    EXPECT_EQ(search.y, 48);
    EXPECT_EQ(search.width, 206);
    EXPECT_EQ(search.height, 28);
    // Wi-Fi opens first; its entry is 34 high and inset by the sidebar margin.
    const RectF wifi = windowRect(window, "sidebar:wlan");
    EXPECT_EQ(wifi.x, 12);
    EXPECT_EQ(wifi.width, 206);
    EXPECT_EQ(wifi.height, 34);
    const RectF page = windowRect(window, "page:wlan");
    EXPECT_EQ(page.x, 230);
    EXPECT_EQ(page.width, 670);
}

TEST(ListRow, KeepsTheRowHeights)
{
    Window window;
    window.setScale(1.5);
    window.resize({400, 300});
    // At its own height, as a page column holds it.
    auto page = std::make_unique<Column>();
    Card* card = page->addChild<Card>(Card::Style::List);
    ListRow* plain = card->addChild<ListRow>("Plain");
    plain->setName("plain");
    InfoRow* empty = card->addChild<InfoRow>("Empty", "");
    empty->setName("empty");
    InfoRow* info = card->addChild<InfoRow>("Info", "Value");
    info->setName("info");
    window.setContentView(std::move(page));
    window.show();
    window.grab();
    EXPECT_EQ(windowRect(window, "plain").height, 44);
    EXPECT_TRUE(isHidden(window, "empty"));
    EXPECT_EQ(windowRect(window, "info").y, 44);
}
