// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <optional>

#include "pagetest.h"
#include "service_thread.h"
#include "touchpad_fake.h"
#include "ui/window.h"
#include "window/settingswindow.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using namespace settings;
using settings::testing::FakeTouchpad;
using settings::testing::ServiceThread;
using settings::testing::windowRect;

namespace {

struct TouchpadTest : ::testing::Test
{
    void SetUp() override
    {
        if (!settings::testing::hasSessionBus())
            GTEST_SKIP() << "no session bus";
        service.emplace("org.cutefish.Touchpad1",
                        [this](dbus::Connection& bus) { return settings::testing::serveTouchpad(bus, fake); });
        ASSERT_TRUE(service->start());
    }

    void TearDown() override
    {
        if (!service)
            return;
        service.reset();
        settings::testing::waitForNameGone("org.cutefish.Touchpad1");
    }

    FakeTouchpad fake;
    std::optional<ServiceThread> service;
};

} // namespace

TEST_F(TouchpadTest, BothCardsLeaveOutTheSecondaryClick)
{
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    settingsWindow.switchToPage("touchpad");
    window.grab();

    // Both cards, the second without the secondary click row.
    EXPECT_EQ(windowRect(window, "touchpad:device").height, 3 * 44);
    const RectF gestures = windowRect(window, "touchpad:gestures");
    EXPECT_EQ(gestures.y, 52 + 3 * 44 + 24);
    EXPECT_EQ(gestures.height, 3 * 44 + 2 * 52);
}
