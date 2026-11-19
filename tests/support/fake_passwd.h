// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdlib.h>
#include <string>
#include <sys/stat.h>

namespace settings::testing {

// Asks as passwd does through PAM on a terminal: the current password is
// "old", a new one of fewer than four characters is refused and asked again.
// What it is finally given goes to `changed` next to it.
inline const char* const kPasswd = R"(#!/bin/sh
[ -t 0 ] && stty -echo || exit 3
[ "$LC_ALL" = C ] || exit 4
printf 'Changing password for test.\n'
printf 'Current password: '
read -r current
printf '\n'
if [ "$current" != old ]; then
    printf 'passwd: Authentication token manipulation error\npasswd: password unchanged\n'
    exit 10
fi
while :; do
    printf 'New password: '
    read -r new
    printf '\n'
    if [ ${#new} -lt 4 ]; then
        printf 'You must choose a longer password.\n'
        continue
    fi
    printf 'Retype new password: '
    read -r again
    printf '\n'
    if [ "$new" != "$again" ]; then
        printf 'Sorry, passwords do not match.\n'
        continue
    fi
    printf '%s' "$new" > "$(dirname "$0")/changed"
    printf 'passwd: password updated successfully\n'
    exit 0
done
)";

// A passwd that asks for the current password and then dies.
inline const char* const kDyingPasswd = R"(#!/bin/sh
printf 'Current password: '
read -r current
kill -KILL $$
)";

// A passwd that asks for the current password and then hangs.
inline const char* const kHangingPasswd = R"(#!/bin/sh
printf 'Current password: '
read -r current
exec sleep 60
)";

class FakePasswd
{
public:
    explicit FakePasswd(const char* script)
    {
        char pattern[] = "/tmp/settings-passwd-XXXXXX";
        m_directory = mkdtemp(pattern);
        path = m_directory + "/passwd";
        std::ofstream(path) << script;
        chmod(path.c_str(), 0755);
    }
    ~FakePasswd() { std::filesystem::remove_all(m_directory); }

    std::string changed() const
    {
        std::ifstream file(m_directory + "/changed");
        return std::string(std::istreambuf_iterator<char>(file), {});
    }

    std::string path;

private:
    std::string m_directory;
};

} // namespace settings::testing
