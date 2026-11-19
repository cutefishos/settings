// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <mutex>
#include <optional>

#include "mouse_fake.h"
#include "scenario.h"
#include "service_thread.h"
#include "widgets/sliderrow.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using namespace settings;
using settings::testing::FakeMouse;
using settings::testing::ServiceThread;

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

TEST_F(MouseTest, PageFollowsAndSetsTheService)
{
    {
        std::lock_guard guard(fake.mutex);
        fake.pointerAcceleration = 0.6;
    }
    ASSERT_TRUE(service->start());
    Scenario scenario("mouse");
    TestDriver& driver = scenario.driver;

    // Speed 0.6 is the seventh of the ten curves after the default.
    const auto* speed = driver.find<SliderRow>("mouse:speed");
    ASSERT_NE(speed, nullptr);
    EXPECT_EQ(speed->value(), 7);

    driver.click("mouse:leftHandedSwitch");
    ASSERT_TRUE(TestDriver::waitFor([&] {
        std::lock_guard guard(fake.mutex);
        return fake.leftHanded;
    }));

    // Without the service the card is there but disabled.
    service->stop();
    const View* card = driver.find("mouse:card");
    ASSERT_NE(card, nullptr);
    ASSERT_TRUE(TestDriver::waitFor([&] { return !card->isEnabled(); }));
}
