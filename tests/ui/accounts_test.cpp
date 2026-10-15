// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include <crypt.h>
#include <memory>
#include <string>

#include "accounts/accountsmanager.h"
#include "pages/accounts/password.h"
#include "pages/accounts/usercard.h"
#include "pages/accounts/userdialogs.h"
#include "ui/dialog_window.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "ui/window.h"
#include "window/settingswindow.h"
#include "input_test_support.h"

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

template<typename T>
std::vector<T*> findAll(View* view)
{
    std::vector<T*> found;
    if (auto* match = dynamic_cast<T*>(view))
        found.push_back(match);
    for (const auto& child : view->children()) {
        for (T* match : findAll<T>(child.get()))
            found.push_back(match);
    }
    return found;
}

PointF centre(View* view)
{
    const PointF origin = view->mapToWindow({0, 0});
    return {origin.x + view->geometry().width / 2, origin.y + view->geometry().height / 2};
}

void typeInto(Window& window, TextField* field, const std::string& text)
{
    TestInput::click(window, centre(field));
    TestInput::typeText(window, text);
}

} // namespace

TEST(Accounts, PasswordsAreSha512WithAFreshSalt)
{
    const std::string first = cryptPassword("secret");
    const std::string second = cryptPassword("secret");
    ASSERT_TRUE(first.starts_with("$6$"));
    EXPECT_NE(first, second);
    crypt_data data = {};
    EXPECT_EQ(std::string(crypt_r("secret", first.c_str(), &data)), first);
}

TEST(Accounts, ChangePasswordWantsTheSamePasswordTwice)
{
    SettingsWindow window;
    ChangePasswordDialog dialog(window, "reion");
    std::string submitted;
    dialog.onPasswordSubmitted = [&](const std::string& password) { submitted = password; };
    dialog.show();
    Window& dialogWindow = *dialog.window();
    dialogWindow.grab();

    const auto fields = findAll<TextField>(dialogWindow.contentView());
    ASSERT_EQ(fields.size(), 2u);
    EXPECT_EQ(fields[0]->echoMode(), TextField::EchoMode::Password);
    const auto buttons = findAll<DialogButton>(dialogWindow.contentView());
    ASSERT_EQ(buttons.size(), 2u);
    DialogButton* change = buttons[1];
    EXPECT_FALSE(change->isEnabled());

    typeInto(dialogWindow, fields[0], "abc");
    typeInto(dialogWindow, fields[1], "abd");
    EXPECT_FALSE(change->isEnabled());
    TestInput::typeKey(dialogWindow, key::BackSpace);
    TestInput::typeText(dialogWindow, "c");
    EXPECT_TRUE(change->isEnabled());
    dialogWindow.grab();
    TestInput::click(dialogWindow, centre(change));
    EXPECT_EQ(submitted, "abc");
    EXPECT_FALSE(dialog.isOpen());
}

TEST(Accounts, AddUserShowsTheServicesAnswer)
{
    SettingsWindow window;
    accounts::AccountsManager manager(nullptr);
    AddUserDialog dialog(window, manager);
    dialog.show();
    Window& dialogWindow = *dialog.window();
    dialogWindow.grab();

    const auto fields = findAll<TextField>(dialogWindow.contentView());
    ASSERT_EQ(fields.size(), 3u);
    const auto buttons = findAll<DialogButton>(dialogWindow.contentView());
    ASSERT_EQ(buttons.size(), 2u);
    DialogButton* add = buttons[1];
    typeInto(dialogWindow, fields[0], "alice");
    typeInto(dialogWindow, fields[1], "pw");
    EXPECT_FALSE(add->isEnabled());
    typeInto(dialogWindow, fields[2], "pw");
    EXPECT_TRUE(add->isEnabled());
    dialogWindow.grab();
    TestInput::click(dialogWindow, centre(add));
    // Without the service the creation fails at once, and the dialog stays.
    EXPECT_TRUE(dialog.isOpen());
    bool shown = false;
    for (Label* label : findAll<Label>(dialogWindow.contentView()))
        shown = shown || (label->isVisible() && label->text() == "The accounts service cannot be reached");
    EXPECT_TRUE(shown);
}

TEST(Accounts, CardUnfoldsToItsDetails)
{
    auto account = std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User1000", nullptr);
    Window window;
    window.setScale(1.5);
    auto root = std::make_unique<Column>();
    root->setMargins({24, 56, 24, 24});
    UserCard* card = root->addChild<UserCard>(account, 1000);
    window.setContentView(std::move(root));
    window.resize({670, 600});
    window.grab(1'000'000'000);
    // The picture and a margin; padded as a card is.
    EXPECT_EQ(card->geometry().height, 84);

    // The card's column, its first row, the header, the header's row.
    View* headerRow = card->children()[0]->children()[0]->children()[0]->children()[0].get();
    // The disclosure button is the row's last child.
    TestInput::click(window, centre(headerRow->children().back().get()));
    window.grab(1'100'000'000);
    window.grab(1'400'000'000);
    // The type on its line below a gap, then the two buttons.
    EXPECT_GT(card->geometry().height, 84 + 12 + 17 + 12 + 36 + 12 + 36 - 1);
}

TEST(Accounts, ThePictureReplacesTheInitialOnceDecoded)
{
    auto account = std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User1000", nullptr);
    Window window;
    window.setScale(1.5);
    auto root = std::make_unique<Column>();
    UserCard* card = root->addChild<UserCard>(account, 1000);
    window.setContentView(std::move(root));
    window.resize({670, 300});
    window.grab();
    const auto initialShown = [&] {
        for (Label* label : findAll<Label>(card)) {
            if (label->text() == "?")
                return label->isVisible();
        }
        return false;
    };
    EXPECT_TRUE(initialShown());

    card->showPicture(std::string(SETTINGS_SOURCE_DIR) + "/images/logo.svg");
    window.grab();
    ASSERT_TRUE(settings::testing::waitFor([&] {
        window.grab();
        return !initialShown();
    }));
}
