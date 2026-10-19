// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>

namespace settings {

// The password as AccountsService takes it: SHA-512 crypt(3) with a fresh salt.
std::string cryptPassword(const std::string& password);

} // namespace settings
