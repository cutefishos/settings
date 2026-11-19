// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <string>
#include <vector>

#include "pages/wallpaper/wallpapers.h"

using namespace settings;

TEST(Wallpapers, SortByFileNameWithNumbersByValue)
{
    std::vector<std::string> paths{"/a/b/img10.jpg", "/a/img2.jpg", "/c/Img1.png", "/b/img2.jpg"};
    wallpapers::sortByFileName(paths);
    EXPECT_EQ(paths, (std::vector<std::string>{"/c/Img1.png", "/a/img2.jpg", "/b/img2.jpg", "/a/b/img10.jpg"}));
    EXPECT_EQ(wallpapers::folderTitle("/home/user/Pictures/"), "Pictures");
    EXPECT_EQ(wallpapers::folderTitle("/srv/walls"), "walls");
}
