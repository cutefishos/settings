// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
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
    EXPECT_TRUE(driver.shows("dock:style"));
    EXPECT_TRUE(driver.shows("dock:displayMode"));
    EXPECT_TRUE(driver.shows("dock:hotcorners"));
    EXPECT_TRUE(driver.shows("dock:minimizedArea"));
}

TEST(DockPage, MagnificationIsOneSliderThatIsOffAtItsLeftEnd)
{
    Scenario scenario("dock");
    TestDriver& driver = scenario.driver;
    // 56 is off, then 64 to 128.
    const auto dragTo = [&](double fraction) {
        driver.dragSlider("dock:magnification", std::min(1.0, fraction + 0.2), fraction);
        return settledValue(driver, "dock:magnification");
    };
    EXPECT_EQ(56, dragTo(0));
    // Just past off it stays off; a little farther it is the smallest size.
    EXPECT_EQ(56, dragTo(2.0 / 72));
    EXPECT_EQ(64, dragTo(6.0 / 72));
    EXPECT_NEAR(92, dragTo(36.0 / 72), 1);
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
