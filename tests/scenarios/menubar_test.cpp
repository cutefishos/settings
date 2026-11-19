// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <chrono>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "scenario.h"
#include "service_thread.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using settings::testing::ServiceThread;

namespace {

struct FakeMenuBar
{
    std::mutex mutex;
    std::vector<std::pair<std::string, bool>> calls;
};

} // namespace

TEST(MenuBarPage, ListsTheControls)
{
    Scenario scenario("menubar");
    TestDriver& driver = scenario.driver;
    EXPECT_EQ(driver.countShown("menubar:check"), 4u);
    for (const char* control : {"wifi", "bluetooth", "battery", "sound"})
        EXPECT_TRUE(driver.shows(std::string("menubar:") + control));
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

    Scenario scenario("menubar");
    TestDriver& driver = scenario.driver;
    // The private bus has no BlueZ: the Bluetooth row is greyed out and
    // takes no clicks, while Sound, which needs no hardware, does.
    const View* bluetooth = driver.find("menubar:bluetooth");
    ASSERT_NE(bluetooth, nullptr);
    ASSERT_TRUE(TestDriver::waitFor([&] { return !bluetooth->isEnabled(); }));
    driver.click("menubar:bluetooth");
    driver.click("menubar:sound");
    ASSERT_TRUE(TestDriver::waitFor([&] {
        std::lock_guard guard(fake.mutex);
        return !fake.calls.empty();
    }));
    TestDriver::waitFor([] { return false; }, std::chrono::milliseconds(100));
    std::lock_guard guard(fake.mutex);
    ASSERT_EQ(fake.calls.size(), 1u);
    EXPECT_EQ(fake.calls[0].first, "sound");
}
