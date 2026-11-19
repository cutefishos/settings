// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "cutefish/accounts/accounts_manager.h"
#include "pages/accounts/picturedialog.h"
#include "pages/accounts/userdetailsdialog.h"
#include "pages/accounts/userdialogs.h"
#include "pages/accounts/userrow.h"
#include "scenario.h"
#include "ui/dialog_window.h"
#include "ui/layout.h"
#include "ui/text_field.h"
#include "ui/window.h"
#include "widgets/card.h"

using namespace cutefish;
using namespace cutefish::ui;
using namespace settings;

static bool isEnabled(TestDriver& driver, const std::string& name)
{
    const View* view = driver.find(name);
    return view && view->isEnabled();
}

TEST(Accounts, ChangePasswordWantsTheSamePasswordTwice)
{
    Scenario scenario;
    ChangePasswordDialog dialog(scenario.settingsWindow(), "reion");
    std::string submitted;
    dialog.onPasswordSubmitted = [&](const std::string& password) { submitted = password; };
    dialog.show();
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);

    // Without a changer of its own, no current password is asked for.
    EXPECT_TRUE(driver->findAll("password:current").empty());
    auto* password = driver->find<TextField>("password:new");
    ASSERT_NE(password, nullptr);
    ASSERT_NE(driver->find<TextField>("password:verify"), nullptr);
    EXPECT_EQ(password->echoMode(), TextField::EchoMode::Password);
    EXPECT_FALSE(isEnabled(*driver, "password:change"));

    driver->click("password:new");
    driver->typeText("abc");
    driver->click("password:verify");
    driver->typeText("abd");
    EXPECT_FALSE(isEnabled(*driver, "password:change"));
    driver->typeKey(key::BackSpace);
    driver->typeText("c");
    EXPECT_TRUE(isEnabled(*driver, "password:change"));
    driver->click("password:change");
    EXPECT_EQ(submitted, "abc");
    EXPECT_FALSE(dialog.isOpen());
}

TEST(Accounts, AddUserShowsTheServicesAnswer)
{
    Scenario scenario;
    accounts::AccountsManager manager(nullptr);
    AddUserDialog dialog(scenario.settingsWindow(), manager);
    dialog.show();
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);

    driver->click("adduser:name");
    driver->typeText("alice");
    driver->click("adduser:password");
    driver->typeText("pw");
    EXPECT_FALSE(isEnabled(*driver, "adduser:add"));
    driver->click("adduser:verify");
    driver->typeText("pw");
    EXPECT_TRUE(isEnabled(*driver, "adduser:add"));
    driver->click("adduser:add");
    // Without the service the creation fails at once, and the dialog stays.
    EXPECT_TRUE(dialog.isOpen());
    EXPECT_TRUE(driver->shows("adduser:error"));
    EXPECT_EQ(driver->text("adduser:error"), "The accounts service cannot be reached");
}

TEST(Accounts, ARowOpensTheDetailsAndThePictureChangesIt)
{
    auto account = std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User1000", nullptr);
    Window window;
    window.setScale(1.5);
    auto root = std::make_unique<Column>();
    root->setMargins({24, 56, 24, 24});
    Card* card = root->addChild<Card>(Card::Style::List);
    card->setLayoutHints({.fillWidth = true});
    UserRow* row = card->addChild<UserRow>(account, 1000);
    window.setContentView(std::move(root));
    window.resize({670, 600});
    window.show();
    TestDriver driver(window, scenarioFailure);
    driver.layout();
    const std::string name = "user:" + account->userName();

    int infoClicks = 0;
    row->onInfo = [&] { ++infoClicks; };
    driver.click(name + ":info");
    // The rest of the row opens the details too.
    driver.click(name);
    EXPECT_EQ(infoClicks, 2);
    // The picture is for changing the picture.
    int pictureClicks = 0;
    row->onPictureClicked = [&] { ++pictureClicks; };
    driver.click(name + ":picture");
    EXPECT_EQ(pictureClicks, 1);
    EXPECT_EQ(infoClicks, 2);
}

TEST(Accounts, TheDetailsShowTheTypeAndLeadToTheActions)
{
    Scenario scenario;
    auto account = std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User1001", nullptr);
    const auto open = [&](int64_t loggedIn) {
        auto details = std::make_unique<UserDetailsDialog>(scenario.settingsWindow(), account, loggedIn);
        details->show();
        return details;
    };

    auto details = open(1000);
    TestDriver driver(*details->dialog()->window(), scenarioFailure);
    driver.layout();
    EXPECT_TRUE(driver.showsText("Standard"));
    EXPECT_TRUE(isEnabled(driver, "user:delete"));

    bool changing = false;
    details->onChangePassword = [&] { changing = true; };
    driver.click("user:changePassword");
    EXPECT_TRUE(changing);

    // The account logged in cannot delete itself.
    auto own = open(1001);
    TestDriver ownDriver(*own->dialog()->window(), scenarioFailure);
    ownDriver.layout();
    EXPECT_FALSE(isEnabled(ownDriver, "user:delete"));
}

TEST(Accounts, ThePictureDialogSavesOnlyAnotherPicture)
{
    Scenario scenario;
    auto account = std::make_shared<accounts::UserAccount>("/org/freedesktop/Accounts/User1001", nullptr);
    PictureDialog dialog(scenario.settingsWindow(), account);
    dialog.show();
    auto driver = scenario.dialog();
    ASSERT_TRUE(driver);
    // Choose File, then Save, which waits for another picture.
    EXPECT_TRUE(driver->shows("picture:chooseFile"));
    EXPECT_TRUE(isEnabled(*driver, "picture:chooseFile"));
    EXPECT_FALSE(isEnabled(*driver, "picture:save"));
}
