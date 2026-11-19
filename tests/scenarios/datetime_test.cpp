// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "cutefish/timedate/time_date.h"
#include "pages/datetime/timezonedialog.h"
#include "pages/datetime/timezonemap.h"
#include "scenario.h"
#include "ui/keys.h"

using namespace cutefish;
using namespace settings;

namespace {

// The map without timedated, so nothing here reaches the system.
struct TimeZoneMapFixture : ::testing::Test
{
    timedate::TimeDate timeDate{nullptr};
    TimeZoneMap map{timeDate};
};

} // namespace

TEST_F(TimeZoneMapFixture, TheDialogPicksAZoneFromTheSearch)
{
    Scenario scenario;
    TimeZoneDialog dialog(scenario.settingsWindow(), map);
    dialog.window()->setScale(1.5);
    dialog.show();
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);

    // The zone in use is selected and its card shown.
    EXPECT_EQ(dialog.selectedZone(), map.currentTimeZoneId());
    EXPECT_FALSE(dialog.isChanged());

    // Typing lists matches; Down moves through them, Return picks.
    driver->click("timezone:search");
    driver->typeText("berl");
    driver->typeKey(ui::key::Return);
    EXPECT_EQ(dialog.selectedZone(), "Europe/Berlin");
    EXPECT_TRUE(dialog.isChanged());
    EXPECT_EQ(driver->text("timezone:search"), "");
}
