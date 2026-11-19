// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "pages/wallpaper/wallpapergrid.h"
#include "pagetest.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace cutefish::ui;
using namespace settings;

TEST(WallpaperGrid, AddsAColumnOnceEveryColumnCanBe128Wide)
{
    WallpaperGrid grid("System");
    grid.setWallpapers(std::vector<std::string>(10, "/x.jpg"));
    // Title, spacing, one row of thumbnails 0.625 as high as wide.
    EXPECT_EQ(grid.heightForWidth(590), 24 + 12 + 81);  // 4 columns of 129.5
    EXPECT_EQ(grid.heightForWidth(500), 24 + 12 + 94);  // 3 of 150.67
    EXPECT_EQ(grid.heightForWidth(300), 24 + 12 + 86);  // 2 of 138
    EXPECT_EQ(grid.heightForWidth(200), 24 + 12 + 100); // 1 of 160
}

TEST(WallpaperGrid, ShowsAllRowsOnRequest)
{
    Window window;
    window.setScale(1.5);
    window.resize({590, 400});
    auto content = std::make_unique<WallpaperGrid>("System");
    WallpaperGrid* grid = content.get();
    grid->setWallpapers(std::vector<std::string>(10, "/x.jpg"));
    window.setContentView(std::move(content));
    window.show();
    window.grab();
    const RectF showAll = settings::testing::windowRect(window, "wallpaper:showAll");
    EXPECT_EQ(showAll.x + showAll.width, 590);
    TestInput::click(window, {showAll.x + showAll.width / 2, showAll.y + showAll.height / 2});
    window.grab();
    // Three rows of 81 with 24 between.
    EXPECT_EQ(grid->heightForWidth(590), 24 + 12 + 3 * 81 + 2 * 24);
}
