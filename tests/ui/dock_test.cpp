// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <chrono>
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
    EXPECT_EQ(page.count("settings::SliderRow"), 2);
    EXPECT_EQ(page.count("SegmentedControl"), 1);
    EXPECT_EQ(page.count("settings::NavigationItem"), 1);
    EXPECT_EQ(page.count("settings::SwitchRow"), 1);
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
