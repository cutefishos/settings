// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "cutefish/timedate/time_date.h"
#include "pages/datetime/timezonedialog.h"
#include "pages/datetime/timezonemap.h"
#include "ui/text_field.h"
#include "ui/window.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace settings;

namespace {

// The map without timedated, so nothing here reaches the system.
struct TimeZoneMapFixture : ::testing::Test
{
    timedate::TimeDate timeDate{nullptr};
    TimeZoneMap map{timeDate};
};

} // namespace

namespace {

// The view of the type with the window coordinates of its rect.
template<typename T>
T* find(ui::View* view)
{
    if (auto* found = dynamic_cast<T*>(view))
        return found;
    for (const auto& child : view->children()) {
        if (T* found = find<T>(child.get()))
            return found;
    }
    return nullptr;
}

ui::RectF windowRect(ui::View* view)
{
    const ui::PointF origin = view->mapToWindow({0, 0});
    return {origin.x, origin.y, view->geometry().width, view->geometry().height};
}

} // namespace

TEST_F(TimeZoneMapFixture, DialogLayout)
{
    SettingsWindow window;
    TimeZoneDialog dialog(window, map);
    dialog.window()->setScale(1.5);
    dialog.show();
    ui::Window& dialogWindow = *dialog.window();
    dialogWindow.grab();
    EXPECT_EQ(dialogWindow.size().width, 900);
    EXPECT_EQ(dialogWindow.size().height, 640);

    // The search box in the middle of the window, on the title bar's band.
    ui::TextField* field = find<ui::TextField>(dialogWindow.contentView());
    ASSERT_TRUE(field);
    const ui::RectF search = windowRect(field->parent());
    EXPECT_EQ(search.x, 310);
    EXPECT_EQ(search.y, 9);
    EXPECT_EQ(search.width, 280);
    EXPECT_EQ(search.height, 30);
}
