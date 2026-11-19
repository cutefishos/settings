// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <optional>

#include "mouse_fake.h"
#include "pages/mouse/mouse.h"
#include "service_thread.h"

using namespace cutefish::base;
using namespace settings;
using settings::testing::FakeMouse;
using settings::testing::ServiceThread;
using settings::testing::waitFor;

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

TEST_F(MouseTest, FollowsTheServiceThroughChangesAndRestarts)
{
    Mouse first;
    Mouse second;
    EXPECT_FALSE(first.isAvailable());

    ASSERT_TRUE(service->start());
    ASSERT_TRUE(waitFor([&] { return first.isAvailable() && second.isAvailable(); }));
    EXPECT_FALSE(second.leftHanded());
    EXPECT_TRUE(second.naturalScroll());

    first.setLeftHanded(true);
    first.setNaturalScroll(false);
    first.setAcceleration(false);
    first.setPointerAcceleration(0.6);
    ASSERT_TRUE(waitFor([&] {
        return second.leftHanded() && !second.naturalScroll() && !second.acceleration()
                && second.pointerAcceleration() == 0.6;
    }));

    service->stop();
    ASSERT_TRUE(waitFor([&] { return !first.isAvailable() && !second.isAvailable(); }));
    // Without the service the switches show the compositor's defaults.
    EXPECT_TRUE(first.acceleration());
    EXPECT_FALSE(first.leftHanded());

    ASSERT_TRUE(service->start());
    ASSERT_TRUE(waitFor([&] { return first.isAvailable() && second.isAvailable(); }));
    EXPECT_TRUE(first.leftHanded());
    EXPECT_FALSE(first.naturalScroll());
    EXPECT_FALSE(first.acceleration());
    EXPECT_EQ(first.pointerAcceleration(), 0.6);

    second.setAcceleration(true);
    ASSERT_TRUE(waitFor([&] { return first.acceleration(); }));
}
