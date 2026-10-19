// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>

namespace settings {

// The icon themes installed for the user and the system.
struct IconTheme
{
    std::string id;
    std::string name;
    // The preview's icons: folder, text, image and trash; empty where the
    // theme and its parents have none.
    std::vector<std::string> previewFiles;

    // Sorted by name; hidden and fallback-only themes left out.
    static std::vector<IconTheme> installed();
};

} // namespace settings
