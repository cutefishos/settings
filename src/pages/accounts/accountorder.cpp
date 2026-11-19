// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/accounts/accountorder.h"

#include <algorithm>

using namespace cutefish;

namespace settings {

std::vector<std::shared_ptr<accounts::UserAccount>>
loggedInFirst(const std::vector<std::shared_ptr<accounts::UserAccount>>& accounts, int64_t loggedInUid)
{
    std::vector<std::shared_ptr<accounts::UserAccount>> ordered = accounts;
    std::stable_partition(ordered.begin(), ordered.end(),
                          [loggedInUid](const auto& account) { return account->userId() == loggedInUid; });
    return ordered;
}

} // namespace settings
