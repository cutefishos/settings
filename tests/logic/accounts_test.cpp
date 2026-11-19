// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <crypt.h>
#include <string>

#include "pages/accounts/password.h"

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
