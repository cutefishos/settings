// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string>

// A key file in a directory of its own, for the proxy settings.
struct KeyFile : ::testing::Test
{
    void SetUp() override
    {
        char pattern[] = "/tmp/proxy-test-XXXXXX";
        directory = mkdtemp(pattern);
        path = directory + "/keyfile";
    }

    void TearDown() override { std::filesystem::remove_all(directory); }

    void write(const std::string& contents)
    {
        std::ofstream file(path);
        file << contents;
    }

    std::string contents() const
    {
        std::ifstream file(path);
        std::stringstream text;
        text << file.rdbuf();
        return text.str();
    }

    std::string directory;
    std::string path;
};
