// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/application_registry.h"
#include "base/event_loop.h"
#include "pages/softwareupdate/softwareupdate.h"
#include "scenario.h"
#include "softwareupdate_fake.h"
#include "ui/application.h"
#include "ui/header_button.h"
#include "ui/icon.h"
#include "ui/progress_bar.h"

using namespace cutefish;
using namespace cutefish::ui;
using namespace settings;
using namespace std::chrono_literals;
using settings::testing::FakeService;

namespace {

bool waitFor(const std::function<bool()>& condition, std::chrono::milliseconds timeout = 5s)
{
    return TestDriver::waitFor(condition, timeout);
}

struct SoftwareUpdatePageTest : ::testing::Test
{
    void SetUp() override
    {
        SoftwareUpdate::setPackageKitBus(base::dbus::Connection::session());
        service = std::make_unique<FakeService>();
    }

    void TearDown() override
    {
        scenario.reset();
        service.reset();
        SoftwareUpdate::setPackageKitBus(nullptr);
    }

    TestDriver& driver() { return scenario->driver; }

    bool openPage()
    {
        scenario.emplace("general");
        if (!waitFor([&] { return inGeneral(); }))
            return false;
        driver().click("general:softwareupdate");
        return driver().shows("page:softwareupdate");
    }

    bool inGeneral() { return driver().shows("general:softwareupdate"); }
    std::string text(const std::string& name) { return driver().shows(name) ? driver().text(name) : std::string(); }
    size_t restarts() const { return size_t(std::count(service->calls.begin(), service->calls.end(), "restart")); }
    // Lets the change reach the page, and lays it out again.
    bool showsTitle(const std::string& title)
    {
        return waitFor([&] {
            driver().layout();
            return text("softwareupdate:title") == title;
        });
    }

    std::unique_ptr<FakeService> service;
    std::optional<Scenario> scenario;
};

} // namespace

TEST_F(SoftwareUpdatePageTest, WithoutPackageKitThereIsNoPage)
{
    scenario.emplace("general");
    EXPECT_TRUE(waitFor([&] { return driver().shows("general:language") && !inGeneral(); }));
}

TEST_F(SoftwareUpdatePageTest, InstallsAndSuggestsARestart)
{
    ASSERT_TRUE(service->ownPackageKit());
    ASSERT_TRUE(openPage());
    ASSERT_TRUE(showsTitle("2 updates available"));
    EXPECT_TRUE(driver().shows("softwareupdate:list"));
    EXPECT_FALSE(driver().shows("softwareupdate:progress"));
    EXPECT_TRUE(driver().shows("softwareupdate:action"));
    EXPECT_EQ(driver().text("softwareupdate:action"), "Update Now");
    // The button beside the list's title checks again.
    EXPECT_TRUE(driver().shows("softwareupdate:check"));
    driver().click("softwareupdate:check");
    ASSERT_TRUE(waitFor([&] { return service->calls == std::vector<std::string>{"check"}; }));
    service->calls.clear();
    driver().click("softwareupdate:action");
    ASSERT_TRUE(waitFor([&] { return service->calls == std::vector<std::string>{"update"}; }));

    service->state = "updating";
    service->progress = 42;
    service->canCancel = true;
    service->change();
    ASSERT_TRUE(showsTitle("Downloading updates… 42%"));
    const auto* bar = driver().find<ProgressBar>("softwareupdate:progress");
    const auto* stop = driver().find<HeaderButton>("softwareupdate:stop");
    ASSERT_TRUE(bar && stop);
    EXPECT_TRUE(driver().shows("softwareupdate:progress"));
    EXPECT_TRUE(driver().shows("softwareupdate:stop"));
    EXPECT_DOUBLE_EQ(bar->value(), 0.42);
    EXPECT_EQ(stop->toolTip(), "Stop");
    EXPECT_FALSE(driver().shows("softwareupdate:detail"));
    EXPECT_FALSE(driver().shows("softwareupdate:action"));
    EXPECT_TRUE(driver().shows("softwareupdate:list"));
    EXPECT_FALSE(driver().shows("softwareupdate:check"));
    driver().click("softwareupdate:stop");
    ASSERT_TRUE(waitFor([&] { return service->calls.back() == "cancel"; }));

    // Installing can no longer be stopped.
    service->phase = "installing";
    service->progress = 80;
    service->canCancel = false;
    service->change();
    ASSERT_TRUE(showsTitle("Installing updates… 80%"));
    EXPECT_FALSE(driver().shows("softwareupdate:stop"));
    EXPECT_TRUE(driver().shows("softwareupdate:progress"));

    service->state = "restart";
    service->updates.clear();
    service->change();
    ASSERT_TRUE(showsTitle("Updates installed"));
    EXPECT_EQ(text("softwareupdate:detail"), "Restart the computer to finish installing them.");
    EXPECT_FALSE(driver().shows("softwareupdate:list"));
    EXPECT_EQ(driver().text("softwareupdate:action"), "Restart");

    // Nothing restarts before the confirmation, and not when it is turned down.
    driver().click("softwareupdate:action");
    auto confirmation = scenario->dialog();
    ASSERT_TRUE(confirmation);
    waitFor([] { return false; }, 100ms);
    EXPECT_EQ(restarts(), 0u);
    confirmation->typeKey(key::Escape);
    confirmation.reset();
    EXPECT_TRUE(waitFor([&] { return driver().dialog() == nullptr; }));
    waitFor([] { return false; }, 100ms);
    EXPECT_EQ(restarts(), 0u);

    driver().click("softwareupdate:action");
    confirmation = scenario->dialog();
    ASSERT_TRUE(confirmation);
    ASSERT_TRUE(confirmation->clickText("Restart"));
    ASSERT_TRUE(waitFor([&] { return restarts() == 1; }));
    waitFor([] { return false; }, 100ms);
    EXPECT_EQ(restarts(), 1u);
}

TEST_F(SoftwareUpdatePageTest, ShowsTheApplicationsIcons)
{
    char directory[] = "/tmp/softwareupdate-apps-XXXXXX";
    ASSERT_TRUE(mkdtemp(directory));
    const auto write = [&](const std::string& file, const std::string& lines) {
        std::ofstream(std::string(directory) + "/" + file) << "[Desktop Entry]\nType=Application\n" << lines;
    };
    // Found by its id, its program and its icon.
    write("firefox.desktop", "Name=Firefox\nExec=firefox %u\nIcon=firefox\n");
    write("org.telegram.desktop.desktop", "Name=Telegram\nExec=telegram-desktop -- %u\nIcon=telegram\n");
    write("com.anthropic.Claude.desktop", "Name=Claude\nExec=claude-desktop %U\nIcon=claude-desktop\n");
    base::EventLoop& loop = Application::instance()->loop();
    base::ApplicationRegistry::setInstance(loop, base::ApplicationRegistry::create(loop, {directory}));

    service->updates = {
        {"claude-desktop", "5.11492.5", "Desktop application for Claude.ai"},
        {"firefox", "160.0~build1", "Mozilla Firefox"},
        {"telegram-desktop", "5.7.2+ds-2~bpo13+2", "fast and secure messaging application"},
        {"tzdata", "2026f-0+deb13u1", "time zone and daylight-saving time data"},
    };
    ASSERT_TRUE(service->ownPackageKit());
    ASSERT_TRUE(openPage());
    ASSERT_TRUE(showsTitle("4 updates available"));
    const auto icon = [&](const std::string& package) {
        const std::vector<ThemeIcon*> icons = driver().findAll<ThemeIcon>("softwareupdate:icon:" + package);
        return icons.size() == 1 ? icons.front()->iconName() : std::string();
    };
    ASSERT_TRUE(waitFor([&] { return icon("firefox") == "firefox"; }));
    EXPECT_EQ(icon("telegram-desktop"), "telegram");
    EXPECT_EQ(icon("claude-desktop"), "claude-desktop");
    EXPECT_EQ(icon("tzdata"), "package-x-generic");

    scenario.reset();
    base::ApplicationRegistry::setInstance(loop, nullptr);
    std::filesystem::remove_all(directory);
}

TEST_F(SoftwareUpdatePageTest, ExplainsEachState)
{
    ASSERT_TRUE(service->ownPackageKit());
    service->state = "checking";
    ASSERT_TRUE(openPage());
    ASSERT_TRUE(showsTitle("Checking for updates…"));
    EXPECT_FALSE(driver().shows("softwareupdate:action"));
    EXPECT_FALSE(driver().shows("softwareupdate:list"));

    service->state = "up-to-date";
    service->updates.clear();
    service->change();
    ASSERT_TRUE(showsTitle("The system is up to date"));
    EXPECT_FALSE(driver().shows("softwareupdate:list"));
    driver().click("softwareupdate:action");
    ASSERT_TRUE(waitFor([&] { return service->calls == std::vector<std::string>{"check"}; }));

    service->state = "error";
    service->error = "Cannot download the package lists";
    service->change();
    ASSERT_TRUE(showsTitle("Cannot check for updates"));
    EXPECT_EQ(text("softwareupdate:detail"), "Cannot download the package lists");
    EXPECT_TRUE(driver().shows("softwareupdate:action"));

    // A failure while updating says so.
    service->state = "updating";
    service->change();
    ASSERT_TRUE(showsTitle("Downloading updates…"));
    service->state = "error";
    service->error = "dpkg returned an error code (1)";
    service->change();
    ASSERT_TRUE(showsTitle("The updates could not be installed"));
}
