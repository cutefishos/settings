// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "cutefish/accounts/user_account.h"

namespace settings {

// The accounts in the service's order, the one logged in first.
std::vector<std::shared_ptr<cutefish::accounts::UserAccount>>
loggedInFirst(const std::vector<std::shared_ptr<cutefish::accounts::UserAccount>>& accounts, int64_t loggedInUid);

} // namespace settings
