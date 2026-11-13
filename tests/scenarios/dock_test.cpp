// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>

#include "scenario.h"
#include "widgets/sliderrow.h"

using namespace cutefish::ui;
using settings::SliderRow;

// The row's value after the release and the settle after the last move,
// when the row puts the handle on the setting's value.
static double settledValue(TestDriver& driver, const std::string& name)
{
    TestDriver::waitFor([] { return false; }, std::chrono::milliseconds(600));
    const auto* row = driver.find<SliderRow>(name);
    return row ? row->value() : -1;
}

static void expectDragHolds(TestDriver& driver, const std::string& name, double fraction)
{
    const auto* row = driver.find<SliderRow>(name);
    ASSERT_NE(row, nullptr);
    const double before = row->value();
    ASSERT_TRUE(driver.dragSlider(name, fraction - 0.1, fraction));
    const double dragged = row->value();
    ASSERT_NE(dragged, before);
    EXPECT_EQ(settledValue(driver, name), dragged);
}

TEST(DockPage, HasEveryControl)
{
    Scenario scenario("dock");
    TestDriver& driver = scenario.driver;
    EXPECT_EQ(driver.countShown("dock:position"), 3u);
    for (const char* slider : {"dock:size", "dock:magnification", "dock:glass"})
        EXPECT_TRUE(driver.shows(slider)) << slider;
    EXPECT_TRUE(driver.shows("dock:displayMode"));
    EXPECT_TRUE(driver.shows("dock:hotcorners"));
    EXPECT_TRUE(driver.shows("dock:minimizedArea"));
}

TEST(DockPage, MagnificationHoldsAtTheIconSizeUntilForcedOff)
{
    Scenario scenario("dock");
    TestDriver& driver = scenario.driver;
    // 32 to 128, as the Size slider: off at 32, else at least the icon size,
    // whatever the configuration it reads holds (54 by default).
    const auto* size = driver.find<SliderRow>("dock:size");
    ASSERT_NE(size, nullptr);
    const double iconSize = size->value();
    if (iconSize < 35 || iconSize > 108)
        GTEST_SKIP() << "No room to drag on both sides of icon size " << iconSize;
    const auto at = [](double value) { return (value - 32) / 96; };
    const double below = std::floor(32 + (iconSize - 32) / 2);
    const auto dragTo = [&](double fraction) {
        driver.dragSlider("dock:magnification", std::min(1.0, fraction + 0.2), fraction);
        return settledValue(driver, "dock:magnification");
    };
    EXPECT_NEAR(iconSize + 20, dragTo(at(iconSize + 20)), 1);
    // Short of the icon size, even at the left end, it holds there; dragged
    // out past the left end it is off.
    EXPECT_EQ(iconSize, dragTo(at(below)));
    EXPECT_EQ(iconSize, dragTo(0));
    EXPECT_EQ(32, dragTo(-0.2));
    // Off, it stays off for a drag that keeps short of the icon size.
    driver.dragSlider("dock:magnification", at(below), at(below - 2));
    EXPECT_EQ(32, settledValue(driver, "dock:magnification"));
}

TEST(DockPage, SizeStaysWhereItIsDragged)
{
    Scenario scenario("dock");
    expectDragHolds(scenario.driver, "dock:size", 0.8);
}

TEST(DockPage, GlassEffectStaysWhereItIsDragged)
{
    Scenario scenario("dock");
    expectDragHolds(scenario.driver, "dock:glass", 0.3);
}
