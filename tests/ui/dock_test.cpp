// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <string>

#include "input_test_support.h"
#include "pagetest.h"
#include "ui/slider.h"
#include "ui/test_input.h"
#include "widgets/sliderrow.h"

using namespace cutefish::ui;
using settings::SliderRow;
using settings::testing::findView;
using settings::testing::waitFor;

namespace {

PointF trackPoint(const Slider& slider, double fraction)
{
    const RectF geometry = slider.geometry();
    const double handle = 22;
    return slider.mapToWindow({handle / 2 + (geometry.width - handle) * fraction, geometry.height / 2});
}

// Drags the row's handle and waits out the release and the settle after the
// last move, when the row puts the handle on the setting's value.
void expectDragHolds(PageFixture& page, const std::string& name, double fraction)
{
    Window& window = page.window->window();
    auto* row = findView<SliderRow>(window.contentView(), name);
    ASSERT_NE(row, nullptr);
    Slider& slider = *row->slider();
    const double before = row->value();

    TestInput::press(window, trackPoint(slider, fraction - 0.1));
    TestInput::movePointer(window, trackPoint(slider, fraction));
    TestInput::release(window, trackPoint(slider, fraction));
    const double dragged = row->value();
    ASSERT_NE(dragged, before);

    waitFor([] { return false; }, std::chrono::milliseconds(600));
    EXPECT_EQ(row->value(), dragged);
}

} // namespace

TEST(DockPage, HasEveryControl)
{
    PageFixture page("dock");
    EXPECT_EQ(page.count("settings::ImageSelectionItem"), 3);
    EXPECT_EQ(page.count("settings::SliderRow"), 3);
    EXPECT_EQ(page.count("SegmentedControl"), 1);
    EXPECT_EQ(page.count("settings::NavigationItem"), 1);
    EXPECT_EQ(page.count("settings::SwitchRow"), 1);
}

TEST(DockPage, MagnificationHoldsAtTheIconSizeUntilForcedOff)
{
    PageFixture page("dock");
    Window& window = page.window->window();
    auto* row = findView<SliderRow>(window.contentView(), "dock:magnification");
    ASSERT_NE(row, nullptr);
    Slider& slider = *row->slider();
    // 32 to 128, as the Size slider: off at 32, else at least the icon size,
    // whatever the configuration it reads holds (54 by default).
    const double iconSize = findView<SliderRow>(window.contentView(), "dock:size")->value();
    if (iconSize < 35 || iconSize > 108)
        GTEST_SKIP() << "No room to drag on both sides of icon size " << iconSize;
    const auto at = [](double value) { return (value - 32) / 96; };
    const double below = std::floor(32 + (iconSize - 32) / 2);
    const auto dragTo = [&](double fraction) {
        TestInput::press(window, trackPoint(slider, std::min(1.0, fraction + 0.2)));
        TestInput::movePointer(window, trackPoint(slider, fraction));
        TestInput::release(window, trackPoint(slider, fraction));
        waitFor([] { return false; }, std::chrono::milliseconds(600));
        return row->value();
    };
    EXPECT_NEAR(iconSize + 20, dragTo(at(iconSize + 20)), 1);
    // Short of the icon size, even at the left end, it holds there; dragged
    // out past the left end it is off.
    EXPECT_EQ(iconSize, dragTo(at(below)));
    EXPECT_EQ(iconSize, dragTo(0));
    EXPECT_EQ(32, dragTo(-0.2));
    // Off, it stays off for a drag that keeps short of the icon size.
    TestInput::press(window, trackPoint(slider, at(below)));
    TestInput::movePointer(window, trackPoint(slider, at(below - 2)));
    TestInput::release(window, trackPoint(slider, at(below - 2)));
    waitFor([] { return false; }, std::chrono::milliseconds(600));
    EXPECT_EQ(32, row->value());
}

TEST(DockPage, SizeStaysWhereItIsDragged)
{
    PageFixture page("dock");
    expectDragHolds(page, "dock:size", 0.8);
}

TEST(DockPage, GlassEffectStaysWhereItIsDragged)
{
    PageFixture page("dock");
    expectDragHolds(page, "dock:glass", 0.3);
}
