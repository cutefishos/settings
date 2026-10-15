// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <string>

#include "ui/keys.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "window/page.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using namespace settings;

namespace {

// The dump line of the view with this name.
std::string dumpLine(const Window& window, const std::string& name)
{
    const std::string dump = window.layoutDump();
    const size_t at = dump.find("\"" + name + "\"");
    if (at == std::string::npos)
        return {};
    const size_t start = dump.rfind('\n', at) + 1;
    return dump.substr(start, dump.find('\n', at) - start);
}

// The view's logical rect in window coordinates, from its dump line.
RectF windowRect(const Window& window, const std::string& name)
{
    const std::string line = dumpLine(window, name);
    const size_t device = line.find(" device ");
    if (device == std::string::npos)
        return {-1, -1, -1, -1};
    long x, y, width, height;
    sscanf(line.c_str() + device, " device %ld %ld %ld %ld", &x, &y, &width, &height);
    const double scale = window.scale();
    return {x / scale, y / scale, width / scale, height / scale};
}

bool isHidden(const Window& window, const std::string& name)
{
    return dumpLine(window, name).ends_with(" hidden");
}

struct Fixture : testing::Test
{
    void SetUp() override
    {
        settingsWindow = std::make_unique<SettingsWindow>();
        window().setScale(1.5);
        window().resize({900, 610});
        settingsWindow->reveal();
        window().grab();
    }

    Window& window() { return settingsWindow->window(); }

    std::unique_ptr<SettingsWindow> settingsWindow;
};

} // namespace

TEST_F(Fixture, SidebarLinesUpWithTheMetrics)
{
    const RectF search = windowRect(window(), "search");
    EXPECT_EQ(search.x, 12);
    EXPECT_EQ(search.y, 56);
    EXPECT_EQ(search.width, 206);
    EXPECT_EQ(search.height, 28);
    // Wi-Fi opens first; its entry is 36 high and inset by the sidebar margin.
    const RectF wifi = windowRect(window(), "sidebar:wlan");
    EXPECT_EQ(wifi.x, 12);
    EXPECT_EQ(wifi.width, 206);
    EXPECT_EQ(wifi.height, 36);
    const RectF page = windowRect(window(), "page:wlan");
    EXPECT_EQ(page.x, 230);
    EXPECT_EQ(page.width, 670);
}

TEST_F(Fixture, HistoryGoesBackAndForward)
{
    EXPECT_FALSE(settingsWindow->canGoBack());
    settingsWindow->switchToPage("about");
    settingsWindow->switchToPage("dock");
    settingsWindow->pushPage("hotcorners");
    window().grab();
    EXPECT_FALSE(isHidden(window(), "page:hotcorners"));
    settingsWindow->goBack();
    window().grab();
    EXPECT_NE(dumpLine(window(), "page:dock"), "");
    settingsWindow->goBack();
    settingsWindow->goBack();
    window().grab();
    EXPECT_NE(dumpLine(window(), "page:wlan"), "");
    EXPECT_FALSE(settingsWindow->canGoBack());
    EXPECT_TRUE(settingsWindow->canGoForward());
    // Going somewhere new drops the steps ahead.
    settingsWindow->switchToPage("sound");
    EXPECT_FALSE(settingsWindow->canGoForward());
}

TEST_F(Fixture, BackAndForwardKeys)
{
    settingsWindow->switchToPage("about");
    TestInput::setKeyboardFocus(window(), true);
    TestInput::typeKey(window(), key::Left, AltModifier);
    window().grab();
    EXPECT_NE(dumpLine(window(), "page:wlan"), "");
    TestInput::typeKey(window(), key::Right, AltModifier);
    window().grab();
    EXPECT_NE(dumpLine(window(), "page:about"), "");
}

TEST_F(Fixture, SubPagesOpenUnderTheirSidebarPage)
{
    settingsWindow->switchToPage("hotcorners");
    window().grab();
    EXPECT_NE(dumpLine(window(), "page:hotcorners"), "");
    // The sidebar keeps Dock current.
    EXPECT_NE(dumpLine(window(), "sidebar:dock"), "");
}

TEST_F(Fixture, SearchMatchesWordStarts)
{
    TestInput::setKeyboardFocus(window(), true);
    TestInput::typeKey(window(), key::letter('f'), ControlModifier);
    TestInput::typeText(window(), "fo");
    window().grab();
    // "fo" finds Fonts, a sub page of Appearance, but not Power's "performance".
    EXPECT_NE(dumpLine(window(), "result:fonts"), "");
    EXPECT_EQ(dumpLine(window(), "result:power"), "");
    TestInput::typeKey(window(), key::Return);
    window().grab();
    EXPECT_NE(dumpLine(window(), "page:fonts"), "");
    // Escape clears the search and shows the sidebar again.
    TestInput::typeKey(window(), key::Escape);
    window().grab();
    EXPECT_FALSE(isHidden(window(), "sidebar:appearance"));
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
