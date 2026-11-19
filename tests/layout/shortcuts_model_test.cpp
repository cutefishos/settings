// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <optional>

#include "pagetest.h"
#include "service_thread.h"
#include "shortcuts_fake.h"
#include "ui/window.h"
#include "window/settingswindow.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using namespace settings;
using settings::testing::FakeShortcuts;
using settings::testing::ServiceThread;
using settings::testing::windowRect;

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

TEST_F(ShortcutsModelTest, PageListsTheActionsInTheirSections)
{
    ASSERT_TRUE(service->start());
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    settingsWindow.pushPage("shortcuts");
    window.grab();
    // One shortcut fits the plain row; two stack, with 4 px between and around them.
    EXPECT_EQ(windowRect(window, "shortcut:lock-screen").height, 44);
    EXPECT_EQ(windowRect(window, "shortcut:toggle-maximized").height, 72);
}
