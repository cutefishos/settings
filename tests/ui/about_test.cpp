// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "pagetest.h"
#include "pages/about/aboutinfo.h"

TEST(AboutPage, ShowsTheSystemRows)
{
    PageFixture page("about");
    EXPECT_NE(page.dump().find("\"page:about\""), std::string::npos);
    // Version, type, kernel, processor, memory and storage all have values here.
    EXPECT_EQ(page.count("settings::InfoRow"), 6);
}

TEST(AboutPage, FormatsSizesAsTheQtVersionDid)
{
    using settings::about::formatByteSize;
    EXPECT_EQ(formatByteSize(512), "512 B");
    EXPECT_EQ(formatByteSize(1536), "2 KB");
    EXPECT_EQ(formatByteSize(16.0 * 1024 * 1024 * 1024), "16 GB");
}
