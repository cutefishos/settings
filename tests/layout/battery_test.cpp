// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

#include "pages/battery/timeformat.h"
#include "pagetest.h"
#include "ui/label.h"
#include "ui/test_input.h"
#include "ui/window.h"
#include "window/options.h"
#include "window/page.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using namespace settings;
using settings::testing::isHidden;
using settings::testing::windowRect;

namespace {

View* findView(View* view, const std::string& name)
{
    if (view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (View* found = findView(child.get(), name))
            return found;
    }
    return nullptr;
}

struct BatteryPageTest : ::testing::Test
{
    virtual Options pageOptions() const { return {.mockBattery = true}; }

    void SetUp() override
    {
        setOptions(pageOptions());
        settingsWindow = std::make_unique<SettingsWindow>();
        window().setScale(1.5);
        window().resize({900, 610});
        settingsWindow->reveal();
        settingsWindow->switchToPage("battery");
        // Through the curve's reveal.
        for (uint64_t ms = 0; ms <= 1000; ms += 100)
            TestInput::presentFrame(window(), ms * 1000000);
        window().grab();
    }

    void TearDown() override { setOptions({}); }

    Window& window() { return settingsWindow->window(); }

    std::string labelText(const std::string& name)
    {
        auto* label = dynamic_cast<Label*>(findView(settingsWindow->currentPage(), name));
        return label ? label->text() : std::string();
    }

    std::unique_ptr<SettingsWindow> settingsWindow;
};

} // namespace

struct TwoBatteriesPageTest : BatteryPageTest
{
    Options pageOptions() const override { return {.mockBattery = true, .mockExtraBattery = true}; }
};

TEST_F(TwoBatteriesPageTest, TheTabsSpanTheCard)
{
    ASSERT_FALSE(isHidden(window(), "battery:tabs"));
    const RectF tabs = windowRect(window(), "battery:tabs");
    const RectF card = windowRect(window(), "battery:graphcard");
    EXPECT_EQ(tabs.x, card.x);
    EXPECT_EQ(tabs.width, card.width);
    EXPECT_LT(tabs.y + tabs.height, windowRect(window(), "battery:history").y);
}

TEST_F(BatteryPageTest, TheRangePickerEndsWithTheCard)
{
    const RectF range = windowRect(window(), "battery:range");
    const RectF card = windowRect(window(), "battery:graphcard");
    // Compact, to sit in the title row.
    EXPECT_EQ(range.height, 32);
    EXPECT_EQ(range.x + range.width, card.x + card.width);
    // Below the title row with the section's label spacing.
    EXPECT_EQ(card.y, range.y + range.height + 8);
    // 240 for the graph and the card's padding.
    EXPECT_EQ(card.height, 240 + 2 * 12);
    const RectF graph = windowRect(window(), "battery:graph");
    EXPECT_EQ(graph.x, card.x + 16);
    EXPECT_EQ(graph.width, card.width - 32);
    EXPECT_EQ(graph.height, 240);
}

TEST_F(BatteryPageTest, TheTitleCentresOnThePicker)
{
    View* page = settingsWindow->currentPage();
    const RectF range = findView(page, "battery:range")->geometry();
    const RectF title = findView(page, "battery:historytitle")->geometry();
    // Centred on the 32 high picker, rounded to a whole pixel. The height is
    // the UI font's line height.
    EXPECT_GT(title.height, 0);
    EXPECT_EQ(title.y, std::round((range.height - title.height) / 2));
    EXPECT_EQ(range.y, 0);
}

TEST_F(BatteryPageTest, ThePlotLeavesRoomForTheAxes)
{
    const RectF graph = windowRect(window(), "battery:graph");
    const RectF plot = windowRect(window(), "graph:plot");
    const RectF legend = windowRect(window(), "graph:legend");
    EXPECT_EQ(plot.x, graph.x);
    EXPECT_GT(plot.y, legend.y + legend.height);
    // The value labels on the right, the time labels below.
    EXPECT_LT(plot.x + plot.width, graph.x + graph.width - 30);
    EXPECT_LT(plot.y + plot.height, graph.y + graph.height - 15);
}

TEST_F(BatteryPageTest, TheHoveringPointerReadsTheCurveOff)
{
    EXPECT_TRUE(isHidden(window(), "graph:readout"));
    const RectF plot = windowRect(window(), "graph:plot");
    TestInput::movePointer(window(), {plot.x + plot.width / 2, plot.y + plot.height / 2});
    window().grab();
    EXPECT_FALSE(isHidden(window(), "graph:readout"));
    // A whole percentage of the sample day, then the state and the time.
    const std::string value = labelText("graph:readout:value");
    ASSERT_FALSE(value.empty());
    EXPECT_EQ(value.back(), '%');
    const int percent = std::stoi(value);
    EXPECT_GE(percent, 26);
    EXPECT_LE(percent, 90);
    EXPECT_NE(labelText("graph:readout:time").find(" · "), std::string::npos);
    const RectF readout = windowRect(window(), "graph:readout");
    const RectF legend = windowRect(window(), "graph:legend");
    EXPECT_EQ(readout.y, legend.y);
    EXPECT_EQ(readout.height, legend.height);

    // Off the plot, the legend comes back.
    TestInput::movePointer(window(), {plot.x + plot.width / 2, plot.y + plot.height + 40});
    window().grab();
    EXPECT_TRUE(isHidden(window(), "graph:readout"));
    TestInput::movePointer(window(), {plot.x + plot.width / 2, plot.y + plot.height / 2});
    TestInput::leavePointer(window());
    window().grab();
    EXPECT_TRUE(isHidden(window(), "graph:readout"));
}

TEST_F(BatteryPageTest, SevenDaysPlotsAWeek)
{
    const RectF range = windowRect(window(), "battery:range");
    TestInput::click(window(), {range.x + range.width * 3 / 4, range.y + range.height / 2});
    for (uint64_t ms = 1000; ms <= 2000; ms += 100)
        TestInput::presentFrame(window(), ms * 1000000);
    window().grab();
    const RectF plot = windowRect(window(), "graph:plot");
    TestInput::movePointer(window(), {plot.x + 1, plot.y + plot.height / 2});
    window().grab();
    // A week's reading names the day.
    const std::string time = labelText("graph:readout:time");
    bool named = false;
    for (int day = 0; day < 7; ++day)
        named = named || time.find(" · " + timeformat::shortDayName(1792000000 + day * 86400) + " ") != std::string::npos;
    EXPECT_TRUE(named) << time;
}

TEST(BatteryRevealTest, TheCurveDrawsInFromTheLeft)
{
    setOptions({.mockBattery = true});
    auto settingsWindow = std::make_unique<SettingsWindow>();
    Window& window = settingsWindow->window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow->reveal();
    settingsWindow->switchToPage("battery");
    // Lays the page out and starts the reveal, then its first tick.
    TestInput::presentFrame(window, 1000000);
    TestInput::presentFrame(window, 2000000);
    const RectF plot = windowRect(window, "graph:plot");
    // The curve's pixels in a column: the line in either colour, against the
    // grey and white of the card and the grid.
    const auto curveAt = [&](const Image& image, double x) {
        int count = 0;
        for (double y = plot.y; y < plot.y + plot.height; y += 1) {
            const Color pixel = image.pixel(int(x * 1.5), int(y * 1.5));
            if (std::max({pixel.r, pixel.g, pixel.b}) - std::min({pixel.r, pixel.g, pixel.b}) > 60)
                ++count;
        }
        return count;
    };
    // Half way through 700 ms of an out-cubic: seven eighths drawn.
    const Image early = TestInput::presentFrame(window, 352000000);
    EXPECT_GT(curveAt(early, plot.x + plot.width * 0.5), 0);
    EXPECT_EQ(curveAt(early, plot.x + plot.width * 0.97), 0);
    const Image done = TestInput::presentFrame(window, 800000000);
    EXPECT_GT(curveAt(done, plot.x + plot.width * 0.97), 0);
    settingsWindow.reset();
    setOptions({});
}

