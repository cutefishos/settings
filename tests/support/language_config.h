// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <filesystem>
#include <stdlib.h>
#include <string>
#include <vector>

#include "base/settings.h"

namespace settings::testing {

// cutefish-services' settings with the given languages, in a directory of its own.
class LanguageConfig
{
public:
    explicit LanguageConfig(const std::vector<std::string>& codes)
    {
        char pattern[] = "/tmp/settings-language-XXXXXX";
        m_directory = mkdtemp(pattern);
        path = m_directory + "/services.conf";
        cutefish::base::Settings settings(path);
        settings.setValue("Language/language", codes.front());
        settings.setValue("Language/preferredLanguages", codes);
    }
    ~LanguageConfig() { std::filesystem::remove_all(m_directory); }

    std::string path;

private:
    std::string m_directory;
};

} // namespace settings::testing
