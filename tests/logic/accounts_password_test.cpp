// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <chrono>
#include <optional>
#include <string>

#include "fake_passwd.h"
#include "pages/accounts/passwordchanger.h"
#include "service_thread.h"

using namespace settings;
using namespace std::chrono_literals;
using settings::testing::FakePasswd;
using settings::testing::kDyingPasswd;
using settings::testing::kHangingPasswd;
using settings::testing::kPasswd;

namespace {

struct Outcome
{
    PasswordChanger::Result result;
    std::string message;
};

std::optional<Outcome> change(const std::string& program, const std::string& current, const std::string& password,
                              std::chrono::milliseconds timeout = 5s)
{
    PasswordChanger changer(program, timeout);
    std::optional<Outcome> outcome;
    changer.onFinished = [&](PasswordChanger::Result result, const std::string& message) {
        outcome = Outcome{result, message};
    };
    if (!changer.start(current, password))
        return std::nullopt;
    EXPECT_TRUE(changer.isRunning());
    settings::testing::waitFor([&] { return outcome.has_value(); }, 5000ms);
    EXPECT_FALSE(changer.isRunning());
    return outcome;
}

} // namespace

TEST(AccountsPassword, ChangesThroughPasswdsPrompts)
{
    FakePasswd passwd(kPasswd);
    const auto outcome = change(passwd.path, "old", "new password");
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::Changed);
    EXPECT_EQ(passwd.changed(), "new password");
}

TEST(AccountsPassword, ReportsAWrongCurrentPassword)
{
    FakePasswd passwd(kPasswd);
    const auto outcome = change(passwd.path, "wrong", "new password");
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::WrongPassword);
    EXPECT_EQ(passwd.changed(), "");
}

TEST(AccountsPassword, ReportsWhyTheNewPasswordWasRefused)
{
    FakePasswd passwd(kPasswd);
    const auto outcome = change(passwd.path, "old", "abc");
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::Rejected);
    EXPECT_EQ(outcome->message, "You must choose a longer password.");
    EXPECT_EQ(passwd.changed(), "");
}

TEST(AccountsPassword, ReportsPasswdDying)
{
    FakePasswd passwd(kDyingPasswd);
    const auto outcome = change(passwd.path, "old", "new password");
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::Failed);
}

TEST(AccountsPassword, GivesUpOnASilentPasswd)
{
    FakePasswd passwd(kHangingPasswd);
    const auto outcome = change(passwd.path, "old", "new password", 300ms);
    ASSERT_TRUE(outcome);
    EXPECT_EQ(outcome->result, PasswordChanger::Result::TimedOut);
}

TEST(AccountsPassword, AMissingPasswdDoesNotStart)
{
    PasswordChanger changer("/nonexistent/passwd");
    EXPECT_FALSE(changer.start("old", "new password"));
    EXPECT_FALSE(changer.isRunning());
}
