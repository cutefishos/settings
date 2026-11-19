// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pages/about/aboutinfo.h"
#include "scenario.h"
#include "ui/keys.h"
#include "ui/text_field.h"

using namespace cutefish::ui;

TEST(AboutPage, ShowsTheSystemRows)
{
    Scenario scenario("about");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("page:about"));
    // Version, type, kernel, processor, memory and storage all have values
    // here; the model row depends on the machine's firmware.
    const size_t modelRows = settings::about::computerModel().empty() ? 0 : 1;
    EXPECT_EQ(driver.countShown("about:info"), 6 + modelRows);
}

TEST(AboutPage, EditsTheDeviceName)
{
    Scenario scenario("about");
    EXPECT_TRUE(scenario.driver.shows("about:name"));
}

TEST(AboutPage, ReturnSelectsTheDeviceName)
{
    Scenario scenario("about");
    TestDriver& driver = scenario.driver;
    auto* field = driver.find<TextField>("about:name");
    ASSERT_NE(field, nullptr);
    ASSERT_FALSE(field->text().empty());
    driver.setKeyboardFocus(true);
    ASSERT_TRUE(driver.focus("about:name"));
    // The name as it is, so nothing is sent to hostnamed.
    driver.typeKey(key::Return);
    EXPECT_TRUE(driver.hasFocus("about:name"));
    EXPECT_EQ(field->selectedText(), field->text());
}

TEST(AboutPage, LinksToTheLicenses)
{
    Scenario scenario("about");
    EXPECT_TRUE(scenario.driver.shows("about:licenses"));
}

TEST(LicensesPage, ListsTheCompositorNotices)
{
    Scenario scenario("about", "licenses");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("page:licenses"));
    EXPECT_EQ(driver.countShown("licenses:notice"), 4u);
    EXPECT_EQ(driver.countShown("licenses:footnote"), 2u);
}
