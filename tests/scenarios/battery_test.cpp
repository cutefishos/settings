// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <optional>

#include "scenario.h"
#include "window/options.h"

using namespace cutefish::ui;
using namespace settings;

namespace {

struct BatteryPageTest : ::testing::Test
{
    virtual Options pageOptions() const { return {.mockBattery = true}; }

    void SetUp() override
    {
        setOptions(pageOptions());
        scenario.emplace("battery");
    }

    void TearDown() override
    {
        scenario.reset();
        setOptions({});
    }

    std::optional<Scenario> scenario;
};

struct TwoBatteriesPageTest : BatteryPageTest
{
    Options pageOptions() const override { return {.mockBattery = true, .mockExtraBattery = true}; }
};

} // namespace

TEST_F(BatteryPageTest, ShowsTheSampleBattery)
{
    TestDriver& driver = scenario->driver;
    EXPECT_TRUE(driver.shows("battery:history"));
    EXPECT_TRUE(driver.shows("battery:health"));
    EXPECT_TRUE(driver.shows("battery:lastcharged"));
    EXPECT_TRUE(driver.shows("battery:charging"));
    // One battery needs no picking.
    const View* tabs = driver.find("battery:tabs");
    ASSERT_NE(tabs, nullptr);
    EXPECT_FALSE(tabs->isVisible());
}

TEST_F(TwoBatteriesPageTest, TheTabsPickTheBatteryShown)
{
    TestDriver& driver = scenario->driver;
    ASSERT_TRUE(driver.shows("battery:tabs"));
    EXPECT_TRUE(driver.showsText("94%"));

    driver.chooseSegment("battery:tabs", 1);
    EXPECT_TRUE(TestDriver::waitFor([&] { return driver.showsText("81%") && !driver.showsText("94%"); }));
    EXPECT_TRUE(driver.showsText("81%"));
    EXPECT_FALSE(driver.showsText("94%"));
    // The charge limit is every battery's.
    EXPECT_TRUE(driver.shows("battery:charging"));
}
