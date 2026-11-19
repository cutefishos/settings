// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>

#include "fake_passwd.h"
#include "pages/accounts/passwordchanger.h"
#include "pages/accounts/userdialogs.h"
#include "scenario.h"
#include "ui/text_field.h"

using namespace cutefish::ui;
using namespace settings;
using namespace std::chrono_literals;
using settings::testing::FakePasswd;
using settings::testing::kPasswd;

TEST(AccountsPassword, TheOwnAccountsDialogAsksTheCurrentPassword)
{
    FakePasswd passwd(kPasswd);
    Scenario scenario;
    ChangePasswordDialog dialog(scenario.settingsWindow(), "reion", std::make_unique<PasswordChanger>(passwd.path));
    dialog.show();
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);

    for (const char* name : {"password:current", "password:new", "password:verify"}) {
        const auto* field = driver->find<TextField>(name);
        ASSERT_NE(field, nullptr) << name;
        EXPECT_EQ(field->echoMode(), TextField::EchoMode::Password) << name;
    }
    const View* change = driver->find("password:change");
    ASSERT_NE(change, nullptr);

    driver->click("password:current");
    driver->typeText("wrong");
    driver->click("password:new");
    driver->typeText("new password");
    driver->click("password:verify");
    driver->typeText("new password");
    EXPECT_TRUE(change->isEnabled());
    driver->click("password:change");
    EXPECT_FALSE(change->isEnabled());
    ASSERT_TRUE(TestDriver::waitFor([&] { return change->isEnabled(); }, 5000ms));
    EXPECT_TRUE(dialog.isOpen());
    EXPECT_TRUE(driver->showsText("The current password is incorrect"));
    EXPECT_EQ(driver->text("password:current"), "");

    driver->click("password:current");
    driver->typeText("old");
    driver->click("password:change");
    ASSERT_TRUE(TestDriver::waitFor([&] { return !dialog.isOpen(); }, 5000ms));
    EXPECT_EQ(passwd.changed(), "new password");
}
