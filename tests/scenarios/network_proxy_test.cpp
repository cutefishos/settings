// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <string>
#include <utility>
#include <vector>

#include "key_file.h"
#include "pages/network/proxy/proxysettings.h"
#include "scenario.h"

using namespace cutefish::ui;
using namespace settings::network;

// A new server and its port saved together: the server going in first must
// not put the old port back into the field before it is read.
TEST_F(KeyFile, ANewServerKeepsTheTypedPort)
{
    write("[system/proxy]\nmode='manual'\n");
    gsettings::setPath(path);
    {
        Scenario scenario("network", "proxy");
        TestDriver& driver = scenario.driver;
        // The script URL, then HTTP, FTP and SOCKS, each with its port.
        for (const char* field : {"proxy:script", "proxy:http", "proxy:httpPort", "proxy:ftp", "proxy:ftpPort",
                                  "proxy:socks", "proxy:socksPort"})
            ASSERT_NE(driver.find(field), nullptr) << field;
        const std::vector<std::pair<std::string, std::string>> typed = {
                {"proxy:http", "http.test"}, {"proxy:httpPort", "3128"},
                {"proxy:socks", "socks.test"}, {"proxy:socksPort", "1081"}};
        for (const auto& [field, text] : typed) {
            driver.click(field);
            driver.typeText(text);
            ASSERT_EQ(driver.text(field), text);
        }

        driver.click("proxy:save");
        EXPECT_EQ(driver.text("proxy:httpPort"), "3128");
        EXPECT_EQ(driver.text("proxy:socksPort"), "1081");
        EXPECT_TRUE(
                TestDriver::waitFor([&] { return contents().find("[system/proxy/socks]") != std::string::npos; }));
    }

    const ProxySettings saved = ProxySettings::load();
    gsettings::setPath({});
    EXPECT_EQ(saved.http, (ProxySettings::Server{"http.test", 3128}));
    EXPECT_EQ(saved.socks, (ProxySettings::Server{"socks.test", 1081}));
}
