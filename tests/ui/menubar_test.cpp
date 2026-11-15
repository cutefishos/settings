// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

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
    EXPECT_EQ(page.count("CheckBox"), 3);
    for (const char* control : {"wifi", "battery", "sound"})
        EXPECT_NE(findView<ListRow>(page.window->window().contentView(), std::string("menubar:") + control), nullptr);
}

TEST(MenuBarPage, AClickOnARowTellsTheMenuBar)
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
    for (const char* control : {"wifi", "battery", "sound"}) {
        auto* row = findView<ListRow>(window.contentView(), std::string("menubar:") + control);
        ASSERT_NE(row, nullptr);
        const RectF rect = row->geometry();
        TestInput::click(window, row->mapToWindow({rect.width / 2, rect.height / 2}));
    }
    ASSERT_TRUE(waitFor([&] {
        std::lock_guard guard(fake.mutex);
        return fake.calls.size() == 3;
    }));
    std::lock_guard guard(fake.mutex);
    EXPECT_EQ(fake.calls[0].first, "wifi");
    EXPECT_EQ(fake.calls[1].first, "battery");
    EXPECT_EQ(fake.calls[2].first, "sound");
}
