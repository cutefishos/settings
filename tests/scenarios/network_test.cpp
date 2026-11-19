// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "scenario.h"
#include "window/options.h"

using namespace cutefish::ui;
using namespace settings;

namespace {

// The sample networks: the pages show without NetworkManager's state.
struct MockNetwork : ::testing::Test
{
    void SetUp() override { setOptions({.mockNetwork = true}); }
    void TearDown() override { setOptions({}); }
};

} // namespace

TEST_F(MockNetwork, WifiSplitsTheNetworks)
{
    Scenario scenario("wlan");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("wlan:switch"));
    // The network in use shares the switch's card.
    EXPECT_TRUE(driver.shows("wifi:Cutefish Studio"));
    EXPECT_LT(driver.frame("wifi:Cutefish Studio").y, driver.frame("wlan:known").y);
    // A saved network is known; the others follow, with the way to add a hidden one last.
    EXPECT_TRUE(driver.shows("wlan:known"));
    EXPECT_GT(driver.frame("wifi:Coffee Shop").y, driver.frame("wlan:known").y);
    EXPECT_LT(driver.frame("wifi:Coffee Shop").y, driver.frame("wlan:other").y);
    EXPECT_GT(driver.frame("wifi:Guest Network").y, driver.frame("wlan:other").y);
    EXPECT_GT(driver.frame("wifi:Neighbor Wi-Fi").y, driver.frame("wifi:Guest Network").y);
    EXPECT_GT(driver.frame("wlan:addOther").y, driver.frame("wifi:Neighbor Wi-Fi").y);
    EXPECT_FALSE(driver.shows("wlan:hotspot"));
}

TEST_F(MockNetwork, EthernetListsTheWiredConnections)
{
    Scenario scenario("ethernet");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("ethernet:switch"));
    EXPECT_TRUE(driver.shows("wired:Wired connection 1"));
    EXPECT_TRUE(driver.shows("wired:USB Ethernet"));
    // The connected one comes first.
    EXPECT_LT(driver.frame("wired:Wired connection 1").y, driver.frame("wired:USB Ethernet").y);
}

TEST(NetworkPage, LeadsToVpnAndProxy)
{
    Scenario scenario("network");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("network:vpn"));
    EXPECT_TRUE(driver.shows("network:proxy"));
    EXPECT_LT(driver.frame("network:vpn").y, driver.frame("network:proxy").y);
}

TEST(VpnPage, OffersAddingAndImporting)
{
    Scenario scenario("network", "vpn");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("vpn:add"));
    EXPECT_TRUE(driver.shows("vpn:import"));
    EXPECT_TRUE(driver.shows("vpn:note"));
    EXPECT_LT(driver.frame("vpn:add").y, driver.frame("vpn:import").y);
    EXPECT_LT(driver.frame("vpn:import").y, driver.frame("vpn:note").y);
}

TEST(HotspotPage, ShowsTheNameAndPassword)
{
    Scenario scenario("network", "hotspot");
    TestDriver& driver = scenario.driver;
    EXPECT_TRUE(driver.shows("hotspot:switch"));
    EXPECT_TRUE(driver.shows("hotspot:status"));
    EXPECT_TRUE(driver.shows("hotspot:name"));
    EXPECT_TRUE(driver.shows("hotspot:password"));
    // The generated name and password are valid, so nothing is wrong yet.
    EXPECT_FALSE(driver.shows("hotspot:validation"));
}
