// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <mutex>
#include <optional>

#include "scenario.h"
#include "service_thread.h"
#include "touchpad_fake.h"
#include "widgets/sliderrow.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using namespace settings;
using settings::testing::FakeTouchpad;
using settings::testing::ServiceThread;

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

TEST_F(TouchpadTest, PageFollowsWhatThePadCanDo)
{
    Scenario scenario("touchpad");
    TestDriver& driver = scenario.driver;
    // Both cards, the second without the secondary click row.
    EXPECT_TRUE(driver.shows("touchpad:device"));
    EXPECT_TRUE(driver.shows("touchpad:gestures"));
    const View* secondaryClick = driver.find("touchpad:secondaryClick");
    ASSERT_NE(secondaryClick, nullptr);
    EXPECT_FALSE(secondaryClick->isVisible());
    // Tap and drag needs tapping on.
    const View* tapDragging = driver.find("touchpad:tapDragging");
    ASSERT_NE(tapDragging, nullptr);
    EXPECT_FALSE(tapDragging->isEnabled());

    // Half of libinput's speed is the fourth of ten steps.
    const auto* scroll = driver.find<SliderRow>("touchpad:scrollSpeed");
    ASSERT_NE(scroll, nullptr);
    EXPECT_EQ(scroll->value(), 3);
    {
        std::lock_guard guard(fake.mutex);
        fake.tapToClick = true;
        fake.scrollSpeed = 1;
    }
    service->post([this] {
        fake.object->emitSignal("tapToClickChanged");
        fake.object->emitSignal("scrollSpeedChanged");
    });
    ASSERT_TRUE(TestDriver::waitFor([&] { return scroll->value() == 6; }));
    EXPECT_TRUE(tapDragging->isEnabled());
}
