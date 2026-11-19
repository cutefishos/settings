// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "bluetooth_fake.h"
#include "cutefish/bluez/manager.h"
#include "scenario.h"
#include "ui/application.h"

using namespace cutefish;
using namespace cutefish::ui;
using namespace settings;
using namespace std::chrono_literals;

namespace {

bool waitFor(const std::function<bool()>& condition)
{
    return TestDriver::waitFor(condition, 5s);
}

// BlueZ on the test's session bus, shared with the page and the sidebar.
struct BluetoothPageTest : ::testing::Test
{
    void SetUp() override
    {
        server = std::make_unique<fake::FakeBluez>(Application::instance()->loop());
        manager = std::make_shared<bluez::Manager>(base::dbus::Connection::session());
        bluez::Manager::share(manager);
    }

    void TearDown() override
    {
        scenario.reset();
        manager.reset();
        server.reset();
    }

    TestDriver& driver() { return scenario->driver; }
    bool inSidebar(const std::string& id) { return !driver().findAll("sidebar:" + id).empty(); }
    void open(const std::string& id)
    {
        scrollTo(driver(), "sidebar:list", "sidebar:" + id);
        driver().click("sidebar:" + id);
    }
    // The named view shows, inside the one named `section`.
    bool showsIn(const std::string& section, const std::string& name)
    {
        if (!driver().shows(section) || !driver().shows(name))
            return false;
        const RectF outer = driver().frame(section);
        const RectF inner = driver().frame(name);
        return inner.x >= outer.x && inner.y >= outer.y && inner.right() <= outer.right()
               && inner.bottom() <= outer.bottom();
    }

    std::unique_ptr<fake::FakeBluez> server;
    std::shared_ptr<bluez::Manager> manager;
    std::optional<Scenario> scenario;
};

} // namespace

TEST_F(BluetoothPageTest, SidebarFollowsTheAdapters)
{
    scenario.emplace();
    TestDriver::waitFor([] { return false; }, 50ms);
    EXPECT_FALSE(manager->available());
    EXPECT_FALSE(inSidebar("bluetooth"));
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor([this] { return manager->available(); }));
    ASSERT_TRUE(waitFor([this] { return inSidebar("bluetooth"); }));
    // Turned off, Bluetooth keeps its entry.
    manager->setEnabled(false);
    ASSERT_TRUE(waitFor([this] { return !manager->enabled(); }));
    EXPECT_TRUE(inSidebar("bluetooth"));
    open("bluetooth");
    const fake::Interfaces adapterProperties = server->objects[{fake::adapter}];
    server->remove(fake::adapter, {fake::adapterInterface});
    ASSERT_TRUE(waitFor([this] { return !inSidebar("bluetooth"); }));
    EXPECT_EQ(driver().text("window:title"), "Wi-Fi");
    server->add(fake::adapter, adapterProperties);
    ASSERT_TRUE(waitFor([this] { return inSidebar("bluetooth"); }));
    server->unregisterService();
    ASSERT_TRUE(waitFor([this] { return !inSidebar("bluetooth"); }));
}

TEST_F(BluetoothPageTest, DiscoversWhileThePageShows)
{
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor([this] { return manager->available(); }));
    scenario.emplace("bluetooth");
    ASSERT_TRUE(waitFor([this] { return manager->scanning(); }));
    // Pairing prompts belong to the shell's agent, not to the page.
    EXPECT_FALSE(manager->agentEnabled());
    open("about");
    ASSERT_TRUE(waitFor([this] { return !manager->scanning(); }));
    open("bluetooth");
    ASSERT_TRUE(waitFor([this] { return manager->scanning(); }));
}

TEST_F(BluetoothPageTest, ListsAdaptersAndDevices)
{
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor([this] { return manager->available(); }));
    scenario.emplace("bluetooth");
    ASSERT_TRUE(waitFor([this] { return manager->scanning(); }));
    driver().layout();
    const std::string adapter = "bluetooth:adapter:" + fake::adapter;
    const std::string device = "bluetooth:device:" + fake::device;
    ASSERT_TRUE(driver().shows(adapter));
    EXPECT_EQ(driver().texts(adapter), (std::vector<std::string>{"Test adapter", "Not discoverable"}));
    EXPECT_FALSE(driver().shows("bluetooth:paired"));
    EXPECT_TRUE(driver().shows("bluetooth:other"));
    ASSERT_TRUE(showsIn("bluetooth:other", device));
    EXPECT_EQ(driver().texts(device), std::vector<std::string>{"Headphones"});
    // While scanning, the title's indicator stands in for the placeholder.
    EXPECT_FALSE(driver().shows("bluetooth:other:placeholder"));

    server->set(fake::device, fake::deviceInterface, "Paired", true);
    ASSERT_TRUE(waitFor([&] { return showsIn("bluetooth:paired", device); }));
    EXPECT_EQ(driver().texts(device), (std::vector<std::string>{"Headphones", "Not connected"}));
    server->set(fake::device, fake::deviceInterface, "Connected", true);
    ASSERT_TRUE(waitFor([&] {
        return driver().texts(device) == std::vector<std::string>{"Headphones", "Connected"};
    }));

    // The other devices' card goes, and the placeholder shows once scanning stops.
    server->set(fake::adapter, fake::adapterInterface, "Discovering", false);
    ASSERT_TRUE(waitFor([&] { return driver().shows("bluetooth:other:placeholder"); }));
    EXPECT_EQ(driver().texts("bluetooth:other:placeholder"),
              std::vector<std::string>{"No devices found. Make sure your device is discoverable."});

    // Turned off, only the switch and the reason stay.
    manager->setEnabled(false);
    ASSERT_TRUE(waitFor([&] { return !driver().shows("bluetooth:paired") && !driver().shows(adapter); }));
}

TEST_F(BluetoothPageTest, ExplainsFailures)
{
    server->failMethod = "StartDiscovery";
    ASSERT_TRUE(server->registerService());
    ASSERT_TRUE(waitFor([this] { return manager->available(); }));
    scenario.emplace("bluetooth");
    ASSERT_NE(driver().find("bluetooth:message"), nullptr);
    ASSERT_TRUE(waitFor([&] { return driver().shows("bluetooth:message"); }));
    EXPECT_EQ(driver().texts("bluetooth:message"), std::vector<std::string>{manager->errorText()});
    EXPECT_EQ(manager->errorText().find("Injected failure"), std::string::npos);
}
