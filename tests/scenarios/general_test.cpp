// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <string>

#include "scenario.h"
#include "ui/keys.h"

using namespace cutefish::ui;

TEST(GeneralPage, LeadsToTheSystemWidePages)
{
    Scenario scenario("general");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("page:general"));
    for (const std::string page : {"about", "datetime", "language", "defaultapps"}) {
        driver.click("general:" + page);
        EXPECT_TRUE(driver.shows("page:" + page)) << page;
        // The sidebar keeps General current.
        EXPECT_TRUE(driver.shows("sidebar:general")) << page;
        driver.click("window:back");
        EXPECT_TRUE(driver.shows("page:general")) << page;
    }
}

TEST(GeneralPage, SitsAboveAppearance)
{
    Scenario scenario;
    TestDriver& driver = scenario.driver;
    scrollTo(driver, "sidebar:list", "sidebar:appearance");
    const RectF general = driver.frame("sidebar:general");
    const RectF appearance = driver.frame("sidebar:appearance");
    EXPECT_LT(general.y, appearance.y);
    EXPECT_TRUE(driver.findAll("sidebar:language").empty());
    EXPECT_TRUE(driver.findAll("sidebar:datetime").empty());
    EXPECT_TRUE(driver.findAll("sidebar:about").empty());
}

TEST(GeneralPage, SearchFindsItsPagesUnderIt)
{
    Scenario scenario;
    TestDriver& driver = scenario.driver;
    driver.setKeyboardFocus(true);
    driver.typeKey(key::letter('f'), ControlModifier);
    driver.typeText("time zone");
    EXPECT_TRUE(driver.shows("result:datetime"));
    driver.typeKey(key::Return);
    EXPECT_TRUE(driver.shows("page:datetime"));
    driver.typeKey(key::Escape);
    EXPECT_TRUE(driver.shows("sidebar:general"));
}

TEST(GeneralPage, NamedPagesOpenUnderIt)
{
    Scenario scenario("language");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("page:language"));
    EXPECT_TRUE(driver.shows("sidebar:general"));
}

TEST(GeneralPage, GroupsTheRows)
{
    Scenario scenario("general");
    TestDriver& driver = scenario.driver;
    // About and Software Update share a card, Date & Time and Language &
    // Region the next, and Default Apps has one of its own.
    const auto frame = [&](const char* name) { return driver.frame(std::string("general:") + name); };
    EXPECT_LT(frame("about").y, frame("datetime").y);
    EXPECT_LT(frame("datetime").y, frame("language").y);
    EXPECT_LT(frame("language").y, frame("defaultapps").y);
    const double inCard = frame("language").y - frame("datetime").y;
    EXPECT_GT(frame("defaultapps").y - frame("language").y, inCard);
    EXPECT_GT(frame("datetime").y - frame("about").y, inCard);
}

TEST(GeneralPage, LicensesOpenUnderIt)
{
    Scenario scenario("about", "licenses");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("page:licenses"));
    EXPECT_TRUE(driver.shows("sidebar:general"));
}
