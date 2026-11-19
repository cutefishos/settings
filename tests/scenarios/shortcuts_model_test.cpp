// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "pages/shortcuts/shortcuteditdialog.h"
#include "pages/shortcuts/shortcutsmodel.h"
#include "scenario.h"
#include "service_thread.h"
#include "shortcuts_fake.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using namespace settings;
using settings::testing::FakeShortcuts;
using settings::testing::ServiceThread;

namespace {

struct ShortcutsModelTest : ::testing::Test
{
    void SetUp() override
    {
        if (!settings::testing::hasSessionBus())
            GTEST_SKIP() << "no session bus";
        service.emplace("org.cutefish.Shortcuts1",
                        [this](dbus::Connection& bus) { return settings::testing::serveShortcuts(bus, fake); });
    }

    void TearDown() override
    {
        if (!service)
            return;
        service.reset();
        settings::testing::waitForNameGone("org.cutefish.Shortcuts1");
    }

    FakeShortcuts fake;
    std::optional<ServiceThread> service;
};

} // namespace

TEST_F(ShortcutsModelTest, NothingToRestoreWhileEveryShortcutIsTheDefault)
{
    ASSERT_TRUE(service->start());
    Scenario scenario("keyboard", "shortcuts");
    TestDriver& driver = scenario.driver;
    ASSERT_TRUE(TestDriver::waitFor([&] { return driver.shows("shortcut:toggle-maximized"); }));
    const View* restoreAll = driver.find("shortcuts:restoreAll");
    ASSERT_NE(restoreAll, nullptr);
    EXPECT_FALSE(restoreAll->isEnabled());
}

TEST_F(ShortcutsModelTest, DialogRecordsAShortcutWithItsKeyCode)
{
    ASSERT_TRUE(service->start());
    Scenario scenario;
    ShortcutsModel model;
    ShortcutEditDialog dialog(scenario.settingsWindow(), model, "toggle-maximized");
    dialog.show();
    dialog.window()->setScale(1.5);
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);
    const View* recorder = driver->find("shortcutdialog:recorder");
    ASSERT_NE(recorder, nullptr);
    EXPECT_FALSE(recorder->isVisible());

    driver->click("shortcutdialog:add");
    EXPECT_TRUE(driver->shows("shortcutdialog:recorder"));
    // Escape alone stops recording and leaves the dialog open.
    driver->pressKey({.key = key::Escape, .keycode = 1});
    EXPECT_FALSE(recorder->isVisible());
    EXPECT_TRUE(dialog.isOpen());

    driver->click("shortcutdialog:add");
    driver->pressKey({.key = 0xffe3, .keycode = 29});
    driver->pressKey({.key = 'q', .keycode = 16, .text = "q", .modifiers = ControlModifier});
    // Sent as the Qt version sent it: the evdev code, Q upper-cased, Ctrl.
    ASSERT_TRUE(TestDriver::waitFor([&] {
        const std::vector<std::string> calls = fake.takeCalls();
        return std::find(calls.begin(), calls.end(), "add toggle-maximized 16 81 2") != calls.end();
    }));
    driver->layout();
    EXPECT_FALSE(recorder->isVisible());
    EXPECT_FALSE(driver->shows("shortcutdialog:message"));

    // A conflict that can be overridden is a warning, and recording goes on.
    {
        std::lock_guard guard(fake.mutex);
        fake.nextResult = "{\"result\":\"conflict-can-override\",\"conflictAction\":\"lock-screen\"}";
    }
    driver->click("shortcutdialog:add");
    driver->pressKey({.key = 'l', .keycode = 38, .text = "l", .modifiers = SuperModifier});
    EXPECT_TRUE(driver->shows("shortcutdialog:recorder"));
    EXPECT_NE(driver->text("shortcutdialog:message").find("Lock the screen"), std::string::npos);
}
