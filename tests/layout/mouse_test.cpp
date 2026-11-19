// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <mutex>
#include <optional>

#include "mouse_fake.h"
#include "pagetest.h"
#include "service_thread.h"
#include "ui/window.h"
#include "window/settingswindow.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using namespace settings;
using settings::testing::FakeMouse;
using settings::testing::ServiceThread;
using settings::testing::windowRect;

namespace {

struct MouseTest : ::testing::Test
{
    void SetUp() override
    {
        if (!settings::testing::hasSessionBus())
            GTEST_SKIP() << "no session bus";
        service.emplace("org.cutefish.Mouse1",
                        [this](dbus::Connection& bus) { return settings::testing::serveMouse(bus, fake); });
    }

    void TearDown() override
    {
        if (!service)
            return;
        service.reset();
        settings::testing::waitForNameGone("org.cutefish.Mouse1");
    }

    FakeMouse fake;
    std::optional<ServiceThread> service;
};

} // namespace

TEST_F(MouseTest, PageLaysOutAsTheQtVersion)
{
    {
        std::lock_guard guard(fake.mutex);
        fake.pointerAcceleration = 0.6;
    }
    ASSERT_TRUE(service->start());
    SettingsWindow settingsWindow;
    Window& window = settingsWindow.window();
    window.setScale(1.5);
    window.resize({900, 610});
    settingsWindow.reveal();
    settingsWindow.switchToPage("mouse");
    window.grab();

    // The list card under the header, three switch rows and a slider row.
    const RectF card = windowRect(window, "mouse:card");
    EXPECT_EQ(card.x, 254);
    EXPECT_EQ(card.y, 52);
    EXPECT_EQ(card.width, 622);
    EXPECT_EQ(card.height, 3 * 44 + 52);
    EXPECT_EQ(windowRect(window, "mouse:speed").height, 52);
}
