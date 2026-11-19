// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <string>

#include "scenario.h"
#include "ui/application.h"
#include "ui/keys.h"

using namespace cutefish::ui;

static bool canGoBack(TestDriver& driver)
{
    const View* back = driver.find("window:back");
    return back && back->isEnabled();
}

static bool canGoForward(TestDriver& driver)
{
    const View* forward = driver.find("window:forward");
    return forward && forward->isEnabled();
}

// The entry scrolled to in the sidebar, then clicked.
static void open(TestDriver& driver, const std::string& entry)
{
    scrollTo(driver, "sidebar:list", entry);
    driver.click(entry);
}

TEST(Foundation, HistoryGoesBackAndForward)
{
    Scenario scenario;
    TestDriver& driver = scenario.driver;
    EXPECT_FALSE(canGoBack(driver));
    open(driver, "sidebar:about");
    open(driver, "sidebar:dock");
    driver.click("dock:hotcorners");
    EXPECT_TRUE(driver.shows("page:hotcorners"));
    driver.click("window:back");
    EXPECT_TRUE(driver.shows("page:dock"));
    driver.click("window:back");
    driver.click("window:back");
    EXPECT_TRUE(driver.shows("page:wlan"));
    EXPECT_FALSE(canGoBack(driver));
    EXPECT_TRUE(canGoForward(driver));
    // Going somewhere new drops the steps ahead.
    open(driver, "sidebar:sound");
    EXPECT_FALSE(canGoForward(driver));
}

TEST(Foundation, LanguageChangeMakesThePageAgain)
{
    Scenario scenario("dock");
    TestDriver& driver = scenario.driver;
    driver.click("dock:hotcorners");
    const View* before = driver.find("page:hotcorners");
    Application::instance()->languageChanged.emit();
    driver.layout();
    EXPECT_TRUE(driver.shows("page:hotcorners"));
    EXPECT_NE(driver.find("page:hotcorners"), before);
    EXPECT_TRUE(canGoBack(driver));
    EXPECT_FALSE(canGoForward(driver));
}

TEST(Foundation, BackAndForwardKeys)
{
    Scenario scenario;
    TestDriver& driver = scenario.driver;
    open(driver, "sidebar:about");
    driver.setKeyboardFocus(true);
    driver.typeKey(key::Left, AltModifier);
    EXPECT_TRUE(driver.shows("page:wlan"));
    driver.typeKey(key::Right, AltModifier);
    EXPECT_TRUE(driver.shows("page:about"));
}

TEST(Foundation, SubPagesOpenUnderTheirSidebarPage)
{
    Scenario scenario("hotcorners");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("page:hotcorners"));
    // The sidebar keeps Dock current.
    EXPECT_TRUE(driver.shows("sidebar:dock"));
}

TEST(Foundation, SearchMatchesWordStarts)
{
    Scenario scenario;
    TestDriver& driver = scenario.driver;
    driver.setKeyboardFocus(true);
    driver.typeKey(key::letter('f'), ControlModifier);
    driver.typeText("fo");
    // "fo" finds Fonts, a sub page of Appearance, but not Power's "performance".
    EXPECT_TRUE(driver.shows("result:fonts"));
    EXPECT_TRUE(driver.findAll("result:power").empty());
    driver.typeKey(key::Return);
    EXPECT_TRUE(driver.shows("page:fonts"));
    // Escape clears the search and shows the sidebar again.
    driver.typeKey(key::Escape);
    EXPECT_TRUE(driver.shows("sidebar:appearance"));
}

TEST(Foundation, SearchFoldsCaseBeyondAscii)
{
    Scenario scenario;
    TestDriver& driver = scenario.driver;
    driver.setKeyboardFocus(true);
    driver.typeKey(key::letter('f'), ControlModifier);
    // The Kelvin sign folds to "k", as "Ü" folds to "ü" in a translated title.
    driver.typeText("Keyb");
    EXPECT_TRUE(driver.shows("result:keyboard"));
}
