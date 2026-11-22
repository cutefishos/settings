// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "cutefish/networkmanager/handler.h"
#include "pages/network/wlan/newnetworkdialog.h"
#include "scenario.h"
#include "widgets/rowcombobox.h"

using namespace cutefish::ui;
using namespace settings;
using namespace settings::network;
namespace nm = cutefish::networkmanager;

namespace {

// The dialog over a settings window, opened as the Wi-Fi page opens it.
struct JoinDialog : ::testing::Test
{
    void SetUp() override
    {
        dialog = std::make_unique<NewNetworkDialog>(scenario.settingsWindow());
        dialog->onJoinNetwork = [this](const std::string& device, const std::string& accessPoint,
                                       const std::string& userName, const std::string& password) {
            joins.push_back({device, accessPoint, userName, password});
        };
        dialog->onConnect = [this](const std::string& ssid, const std::string& userName, const std::string& password,
                                   const std::string& type) { connects.push_back({ssid, userName, password, type}); };
    }

    void TearDown() override { dialog.reset(); }

    bool canJoin(TestDriver& driver)
    {
        const View* join = driver.find("join:join");
        return join && join->isEnabled();
    }

    std::string security(TestDriver& driver)
    {
        const auto* box = driver.find<settings::RowComboBox>("join:security");
        return box ? box->currentText() : std::string();
    }

    bool closed() { return TestDriver::waitFor([this] { return !scenario.driver.dialog(); }); }

    const nm::WirelessNetwork library{"/org/freedesktop/NetworkManager/Devices/2", "Library", nm::SecurityType::Wpa2Eap};
    const std::string libraryAccessPoint = "/org/freedesktop/NetworkManager/AccessPoint/5";

    Scenario scenario;
    std::unique_ptr<NewNetworkDialog> dialog;
    std::vector<std::vector<std::string>> joins;
    std::vector<std::vector<std::string>> connects;
};

} // namespace

TEST_F(JoinDialog, JoinsAListedEnterpriseNetwork)
{
    dialog->openNetwork(libraryAccessPoint, library);
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);
    EXPECT_TRUE(driver->showsText("Join “Library”"));
    EXPECT_EQ(security(*driver), "WPA2 Enterprise");

    // The network is known; only the sign-in is asked for, user name first.
    EXPECT_FALSE(driver->shows("join:name"));
    EXPECT_FALSE(driver->shows("join:security"));
    EXPECT_TRUE(driver->shows("join:userName"));
    EXPECT_TRUE(driver->shows("join:password"));
    EXPECT_TRUE(driver->hasFocus("join:userName"));

    EXPECT_FALSE(canJoin(*driver));
    replaceText(*driver, "join:userName", "alice");
    EXPECT_FALSE(canJoin(*driver));
    replaceText(*driver, "join:password", "secret");
    EXPECT_TRUE(canJoin(*driver));

    driver->click("join:join");
    ASSERT_EQ(joins.size(), 1u);
    EXPECT_EQ(joins[0], (std::vector<std::string>{library.device, libraryAccessPoint, "alice", "secret"}));
    EXPECT_TRUE(connects.empty());
    driver.reset();
    EXPECT_TRUE(closed());
}

TEST_F(JoinDialog, WpaEnterpriseKeepsItsKind)
{
    dialog->openNetwork(libraryAccessPoint, nm::WirelessNetwork{library.device, "Old", nm::SecurityType::WpaEap});
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);
    EXPECT_EQ(security(*driver), "WPA Enterprise");
}

TEST_F(JoinDialog, CertificateNetworkCannotBeJoined)
{
    dialog->openNetwork(libraryAccessPoint, nm::WirelessNetwork{library.device, "Secure", nm::SecurityType::Wpa3SuiteB192});
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);
    EXPECT_TRUE(driver->shows("join:certificate"));
    EXPECT_FALSE(driver->shows("join:userName"));
    EXPECT_FALSE(driver->shows("join:password"));

    EXPECT_FALSE(canJoin(*driver));
    driver->click("join:join");
    EXPECT_TRUE(joins.empty());
}

TEST_F(JoinDialog, GoneNetworkOpensNothing)
{
    // Handler::wirelessNetwork() is empty once the access point has gone.
    dialog->openNetwork(libraryAccessPoint, std::nullopt);
    scenario.driver.layout();
    EXPECT_EQ(scenario.driver.dialog(), nullptr);
}

TEST_F(JoinDialog, ClosingReturnsToHiddenNetworks)
{
    dialog->openNetwork(libraryAccessPoint, library);
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);
    replaceText(*driver, "join:userName", "alice");
    driver->typeKey(key::Escape);
    driver.reset();
    ASSERT_TRUE(closed());

    dialog->show();
    driver = scenario.dialog();
    ASSERT_TRUE(driver);
    EXPECT_TRUE(driver->showsText("Add Other Network"));
    EXPECT_EQ(security(*driver), "WPA2/WPA3");
    EXPECT_EQ(driver->text("join:userName"), "");
    EXPECT_TRUE(driver->shows("join:name"));
    EXPECT_TRUE(driver->shows("join:security"));
    EXPECT_TRUE(driver->hasFocus("join:name"));
    replaceText(*driver, "join:name", "Garden");
    replaceText(*driver, "join:password", "garden pass");
    driver->click("join:join");
    ASSERT_EQ(connects.size(), 1u);
    EXPECT_EQ(connects[0], (std::vector<std::string>{"Garden", "", "garden pass", "wpa2-wpa3"}));
    EXPECT_TRUE(joins.empty());
}

TEST_F(JoinDialog, PasswordsFitTheSecurity)
{
    dialog->show();
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);
    replaceText(*driver, "join:name", "Garden");
    // WPA2/WPA3 passphrases are 8 to 63 characters.
    replaceText(*driver, "join:password", "short");
    EXPECT_FALSE(canJoin(*driver));
    replaceText(*driver, "join:password", "long enough");
    EXPECT_TRUE(canJoin(*driver));
    // 64 characters are the key itself, in hexadecimal.
    replaceText(*driver, "join:password", std::string(64, 'a'));
    EXPECT_TRUE(canJoin(*driver));
    replaceText(*driver, "join:password", std::string(64, 'z'));
    EXPECT_FALSE(canJoin(*driver));
    // WEP keys come in four lengths, the longer two hexadecimal; an open network needs none.
    driver->click("join:security");
    driver->choosePopupItem("WEP");
    EXPECT_FALSE(canJoin(*driver));
    replaceText(*driver, "join:password", "abcde");
    EXPECT_TRUE(canJoin(*driver));
    replaceText(*driver, "join:password", "0123456789");
    EXPECT_TRUE(canJoin(*driver));
    replaceText(*driver, "join:password", "garden key");
    EXPECT_FALSE(canJoin(*driver));
    driver->click("join:security");
    driver->choosePopupItem("Open");
    EXPECT_TRUE(canJoin(*driver));
}
