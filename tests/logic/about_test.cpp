// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "pages/about/aboutinfo.h"
#include "pages/about/devicename.h"

TEST(AboutPage, NamesTheDeviceAsHostnamedDoes)
{
    using settings::DeviceName;
    EXPECT_EQ(DeviceName::chooseName("Kai’s Laptop", "reion-pc", "localhost"), "Kai’s Laptop");
    EXPECT_EQ(DeviceName::chooseName("", "reion-pc", "localhost"), "reion-pc");
    EXPECT_EQ(DeviceName::chooseName("", "", "localhost"), "localhost");
}

TEST(AboutPage, FormatsSizesAsTheQtVersionDid)
{
    using settings::about::formatByteSize;
    EXPECT_EQ(formatByteSize(512), "512 B");
    EXPECT_EQ(formatByteSize(1536), "2 KB");
    EXPECT_EQ(formatByteSize(16.0 * 1024 * 1024 * 1024), "16 GB");
}

TEST(AboutPage, NamesTheComputerModel)
{
    using settings::about::computerModel;
    EXPECT_EQ(computerModel("VENDOR", "Book 13", "AB-1234", ""), "VENDOR Book 13");
    EXPECT_EQ(computerModel("Vendor Inc.", "Vendor Book", "", ""), "Vendor Book");
    EXPECT_EQ(computerModel("Vendor", "To be filled by O.E.M.", "Book 14", ""), "Vendor Book 14");
    EXPECT_EQ(computerModel("System manufacturer", "", "System Product Name", ""), "");
    EXPECT_EQ(computerModel("", "", "", "Board Model B"), "Board Model B");
    EXPECT_EQ(computerModel("Default string", "Default string", "Default string", ""), "");
}
