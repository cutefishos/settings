// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <optional>
#include <string>

#include "scenario.h"
#include "widgets/listrow.h"
#include "window/options.h"

using namespace cutefish::ui;
using namespace settings;

namespace {

struct PowerPageTest : ::testing::Test
{
    void open(bool battery)
    {
        setOptions({.mockBattery = battery});
        scenario.emplace("power");
    }

    void TearDown() override
    {
        scenario.reset();
        setOptions({});
    }

    std::string rowText(const std::string& name)
    {
        const auto* row = scenario->driver.find<ListRow>(name);
        return row ? row->text() : std::string();
    }

    std::optional<Scenario> scenario;
};

} // namespace

TEST_F(PowerPageTest, OnBatteryEachSourceHasItsTimeout)
{
    open(true);
    EXPECT_TRUE(scenario->driver.shows("power:batteryscreenoff"));
    EXPECT_EQ(rowText("power:batteryscreenoff"), "Turn off display when inactive on battery");
    EXPECT_EQ(rowText("power:acscreenoff"), "Turn off display when inactive on power adapter");
    EXPECT_EQ(rowText("power:preventsleep"), "Prevent automatic sleeping on power adapter when the display is off");
}

TEST_F(PowerPageTest, WithoutABatteryThereIsOneTimeout)
{
    // The battery service has not answered here, so there is none.
    open(false);
    const View* battery = scenario->driver.find("power:batteryscreenoff");
    ASSERT_NE(battery, nullptr);
    EXPECT_FALSE(battery->isVisible());
    EXPECT_EQ(rowText("power:acscreenoff"), "Turn off display when inactive");
    EXPECT_EQ(rowText("power:preventsleep"), "Prevent automatic sleeping when the display is off");
}

TEST_F(PowerPageTest, ModesWaitForTheDaemon)
{
    open(true);
    // No profiles before power-profiles-daemon answers.
    const View* mode = scenario->driver.find("power:mode");
    ASSERT_NE(mode, nullptr);
    EXPECT_FALSE(mode->isVisible());
}
