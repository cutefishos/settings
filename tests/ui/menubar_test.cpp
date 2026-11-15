// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <chrono>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "input_test_support.h"
#include "pagetest.h"
#include "ui/test_input.h"
#include "widgets/listrow.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using settings::ListRow;
using settings::testing::findView;
using settings::testing::ServiceThread;
using settings::testing::waitFor;

namespace {

struct FakeMenuBar
{
    std::mutex mutex;
    std::vector<std::pair<std::string, bool>> calls;
};

} // namespace

TEST(MenuBarPage, ListsTheControls)
{
    PageFixture page("menubar");
    EXPECT_EQ(page.count("CheckBox"), 4);
    for (const char* control : {"wifi", "bluetooth", "battery", "sound"})
        EXPECT_NE(findView<ListRow>(page.window->window().contentView(), std::string("menubar:") + control), nullptr);
}

TEST(MenuBarPage, AClickOnAnAvailableRowTellsTheMenuBar)
{
    if (!settings::testing::hasSessionBus())
        GTEST_SKIP() << "no session bus";
    FakeMenuBar fake;
    ServiceThread service("org.cutefish.MenuBar", [&fake](dbus::Connection& bus) {
        auto object = std::make_shared<dbus::Object>(&bus, "/MenuBar", "org.cutefish.MenuBar");
        object->addMethod("setControlShown", [&fake](const std::string& control, bool shown) {
            std::lock_guard guard(fake.mutex);
            fake.calls.emplace_back(control, shown);
        });
        object->publish();
        return object;
    });
    ASSERT_TRUE(service.start());

    PageFixture page("menubar");
    Window& window = page.window->window();
    const auto click = [&](const std::string& control) {
        auto* row = findView<ListRow>(window.contentView(), "menubar:" + control);
        ASSERT_NE(row, nullptr);
        const RectF rect = row->geometry();
        TestInput::click(window, row->mapToWindow({rect.width / 2, rect.height / 2}));
    };
    // The private bus has no BlueZ: the Bluetooth row is greyed out and
    // takes no clicks, while Sound, which needs no hardware, does.
    auto* bluetooth = findView<ListRow>(window.contentView(), "menubar:bluetooth");
    ASSERT_NE(bluetooth, nullptr);
    ASSERT_TRUE(waitFor([&] { return !bluetooth->isEnabled(); }));
    click("bluetooth");
    click("sound");
    ASSERT_TRUE(waitFor([&] {
        std::lock_guard guard(fake.mutex);
        return !fake.calls.empty();
    }));
    waitFor([] { return false; }, std::chrono::milliseconds(100));
    std::lock_guard guard(fake.mutex);
    ASSERT_EQ(fake.calls.size(), 1u);
    EXPECT_EQ(fake.calls[0].first, "sound");
}
