// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "cutefish/accounts/account_picture.h"
#include "cutefish/accounts/user_account.h"
#include "pages/accounts/userrow.h"
#include "service_thread.h"
#include "ui/image_view.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/iconbutton.h"

using namespace cutefish;
using namespace cutefish::ui;
using namespace settings;

namespace {

template<typename T>
T* find(View* view)
{
    if (auto* found = dynamic_cast<T*>(view))
        return found;
    for (const auto& child : view->children()) {
        if (T* found = find<T>(child.get()))
            return found;
    }
    return nullptr;
}

PointF centre(View* view)
{
    const PointF origin = view->mapToWindow({0, 0});
    return {origin.x + view->geometry().width / 2, origin.y + view->geometry().height / 2};
}

// One account's row in a list card, as the Users page shows it.
struct UserRowFixture
{
    UserRowFixture()
    {
        window.setScale(1.5);
        auto root = std::make_unique<Column>();
        root->setMargins({24, 56, 24, 24});
        Card* card = root->addChild<Card>(Card::Style::List);
        card->setLayoutHints({.fillWidth = true});
        row = card->addChild<UserRow>(account, 1000);
        window.setContentView(std::move(root));
        window.resize({670, 600});
        window.grab(1'000'000'000);
    }

    View* avatar() const { return find<ImageView>(row)->parent(); }

    std::shared_ptr<accounts::UserAccount> account =
            std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User1000", nullptr);
    Window window;
    UserRow* row = nullptr;
};

} // namespace

TEST(Accounts, ARowIsThePictureAndNamesWithAnInfoButton)
{
    UserRowFixture fixture;
    // As much room above and below the picture as before it.
    const RectF row = fixture.row->geometry();
    const PointF rowOrigin = fixture.row->mapToWindow({0, 0});
    const PointF pictureOrigin = fixture.avatar()->mapToWindow({0, 0});
    EXPECT_EQ(row.height, 48 + 2 * 16);
    EXPECT_EQ(pictureOrigin.x - rowOrigin.x, 16);
    EXPECT_EQ(pictureOrigin.y - rowOrigin.y, 16);
    EXPECT_EQ(rowOrigin.y + row.height - (pictureOrigin.y + 48), 16);
    IconButton* info = find<IconButton>(fixture.row);
    ASSERT_TRUE(info);
    // At the end of the row, centred on it as the picture is.
    const PointF picture = centre(fixture.avatar());
    const PointF button = centre(info);
    EXPECT_DOUBLE_EQ(picture.y, button.y);
    EXPECT_GT(button.x, 670 - 24 - 16 - 22);
}

TEST(Accounts, HoveringThePictureDimsItUnderAPencil)
{
    UserRowFixture fixture;
    Window& window = fixture.window;
    View* avatar = fixture.avatar();
    const PointF middle = centre(avatar);
    // Inside the circle, above the pencil.
    const PointF probe {middle.x, middle.y - 18};
    const auto lightness = [&](const Image& image) {
        const Color pixel = image.pixel(int(probe.x * 1.5), int(probe.y * 1.5));
        return pixel.r + pixel.g + pixel.b;
    };
    const int plain = lightness(window.grab(1'000'000'000));

    TestInput::movePointer(window, middle);
    window.grab(1'050'000'000);
    const Image hovered = window.grab(1'400'000'000);
    EXPECT_LT(lightness(hovered), plain);

    TestInput::leavePointer(window);
    window.grab(1'450'000'000);
    EXPECT_EQ(lightness(window.grab(1'800'000'000)), plain);
}

TEST(Accounts, ThePictureReplacesTheAccountColourOnceDecoded)
{
    UserRowFixture fixture;
    Window& window = fixture.window;
    const PointF middle = centre(fixture.avatar());
    const auto pixel = [&] { return window.grab().pixel(int(middle.x * 1.5), int(middle.y * 1.5)); };
    EXPECT_EQ(pixel(), Color::rgb(accounts::pictureColor(fixture.account->userName())));

    fixture.row->showPicture(std::string(SETTINGS_SOURCE_DIR) + "/images/logo.svg");
    ASSERT_TRUE(settings::testing::waitFor([&] {
        return pixel() != Color::rgb(accounts::pictureColor(fixture.account->userName()));
    }));
}
