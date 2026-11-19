// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <crypt.h>
#include <memory>
#include <string>
#include <vector>

#include "pages/accounts/accountorder.h"
#include "pages/accounts/password.h"

using namespace cutefish;
using namespace settings;

TEST(Accounts, PasswordsAreSha512WithAFreshSalt)
{
    const std::string first = cryptPassword("secret");
    const std::string second = cryptPassword("secret");
    ASSERT_TRUE(first.starts_with("$6$"));
    EXPECT_NE(first, second);
    crypt_data data = {};
    EXPECT_EQ(std::string(crypt_r("secret", first.c_str(), &data)), first);
}

TEST(Accounts, TheLoggedInAccountComesFirst)
{
    const auto make = [](int uid) {
        return std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User" + std::to_string(uid),
                                                       nullptr);
    };
    const std::vector<std::shared_ptr<accounts::UserAccount>> accounts {make(1001), make(1002), make(1000),
                                                                       make(1003)};
    std::vector<int64_t> uids;
    for (const auto& account : loggedInFirst(accounts, 1000))
        uids.push_back(account->userId());
    EXPECT_EQ(uids, (std::vector<int64_t> {1000, 1001, 1002, 1003}));
}
