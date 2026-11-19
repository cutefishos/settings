// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <optional>
#include <vector>

#include "pages/battery/batteryhistory.h"
#include "pages/battery/timeformat.h"

using cutefish::upower::HistorySample;
using namespace settings;

TEST(BatteryHistoryTest, StartsWithTheValueInForce)
{
    const uint32_t now = 100000;
    // Newest first, as UPower returns them.
    const std::vector<HistorySample> window{{99000, 50, 2}, {95000, 60, 2}};
    const std::vector<HistorySample> all{{99000, 50, 2}, {95000, 60, 2}, {90000, 70, 1}, {80000, 75, 1}};
    const auto values = BatteryHistory::plotted(window, all, now, 10000);
    ASSERT_EQ(values.size(), 4u);
    // The sample before the window, moved to its start.
    EXPECT_EQ(values[0].time, 90000u);
    EXPECT_EQ(values[0].value, 70);
    EXPECT_EQ(values[1].time, 95000u);
    EXPECT_EQ(values[2].time, 99000u);
    // The last value runs on to the present.
    EXPECT_EQ(values[3].time, now);
    EXPECT_EQ(values[3].value, 50);
}

TEST(BatteryHistoryTest, SkipsRestartsWrongClocksAndDuplicates)
{
    const uint32_t now = 100000;
    // Newest first, as UPower returns them.
    const std::vector<HistorySample> window{
        {99500, 40, 2},
        // From the future.
        {200000, 45, 2},
        // Stamped before the clock was set: older than the one recorded before it.
        {96500, 58, 2},
        // Two in one second: the later one wins.
        {97000, 56, 2},
        {97000, 55, 2},
        // A restart marker.
        {96000, 0, 0},
        {95000, 60, 2},
    };
    const auto values = BatteryHistory::plotted(window, std::nullopt, now, 10000);
    ASSERT_EQ(values.size(), 4u);
    EXPECT_EQ(values[0].time, 95000u);
    EXPECT_EQ(values[0].value, 60);
    EXPECT_EQ(values[1].time, 97000u);
    EXPECT_EQ(values[1].value, 56);
    EXPECT_EQ(values[2].time, 99500u);
    EXPECT_EQ(values[3].time, now);
    EXPECT_EQ(values[3].value, 40);
}

TEST(BatteryHistoryTest, AnEmptyWindowPlotsNothing)
{
    EXPECT_TRUE(BatteryHistory::plotted({}, std::nullopt, 100000, 86400).empty());
}

TEST(TimeFormatTest, CountsLocalDays)
{
    const int64_t noon = 1792000000;
    EXPECT_EQ(timeformat::daysBetween(noon, noon), 0);
    EXPECT_EQ(timeformat::daysBetween(noon - 86400, noon), 1);
    EXPECT_EQ(timeformat::daysBetween(noon - 3 * 86400, noon), 3);
}
