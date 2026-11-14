// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "base/application_registry.h"
#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/timer.h"
#include "pages/softwareupdate/softwareupdate.h"
#include "ui/application.h"
#include "ui/button.h"
#include "ui/header_button.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/progress_bar.h"
#include "ui/window.h"
#include "widgets/listrow.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;
using namespace settings;
using namespace std::chrono_literals;

namespace {

bool waitFor(const std::function<bool()>& condition, std::chrono::milliseconds timeout = 5s)
{
    base::EventLoop& loop = Application::instance()->loop();
    if (condition())
        return true;
    base::Timer poll(&loop);
    poll.onTimeout = [&] {
        if (condition())
            loop.quit();
    };
    base::Timer limit(&loop);
    limit.setSingleShot(true);
    limit.onTimeout = [&loop] { loop.quit(); };
    poll.start(5ms);
    limit.start(timeout);
    loop.run();
    return condition();
}

View* findView(View* view, const std::string& name)
{
    if (!view)
        return nullptr;
    if (view->name() == name)
        return view;
    for (const auto& child : view->children()) {
        if (View* found = findView(child.get(), name))
            return found;
    }
    return nullptr;
}

bool isShown(View* view)
{
    for (; view; view = view->parent()) {
        if (!view->isVisible())
            return false;
    }
    return true;
}

using UpdateEntry = std::tuple<std::string, std::string, std::string>;

// cutefish-services' software update object, and PackageKit's name when
// the test wants it, on a connection of their own on the test's session bus.
class FakeService
{
public:
    FakeService()
        : m_bus(base::dbus::Connection::open(base::dbus::Connection::Bus::Session,
                                             &Application::instance()->loop()))
    {
        m_object = std::make_unique<base::dbus::Object>(m_bus.get(), "/org/cutefish/Services/SoftwareUpdate",
                                                        "org.cutefish.Services.SoftwareUpdate");
        m_object->addProperty("state", [this] { return state; });
        m_object->addProperty("updates", [this] { return updates; });
        m_object->addProperty("progress", [this] { return progress; });
        m_object->addProperty("error", [this] { return error; });
        m_object->addMethod("check", [this] { calls.push_back("check"); });
        m_object->addMethod("download", [this] { calls.push_back("download"); });
        m_object->addMethod("cancel", [this] { calls.push_back("cancel"); });
        m_object->addMethod("restartAndUpdate", "", "", [this](base::dbus::MethodCall& call) {
            calls.push_back("restartAndUpdate");
            call.replyError("org.cutefish.Services.SoftwareUpdate.Error.Failed", "Not authorized");
        });
        EXPECT_TRUE(m_object->publish());
        EXPECT_TRUE(m_bus->requestName("org.cutefish.Services"));
    }

    bool ownPackageKit() { return m_bus->requestName("org.freedesktop.PackageKit"); }

    void change() { m_object->emitPropertiesChanged({"state", "updates", "progress", "error"}); }

    std::string state = "available";
    std::vector<UpdateEntry> updates = {
        {"openssl", "3.5.4-1~deb13u2", "Secure Sockets Layer toolkit"},
        {"tzdata", "2026f-0+deb13u1", "time zone and daylight-saving time data"},
    };
    int32_t progress = -1;
    std::string error;
    std::vector<std::string> calls;

private:
    std::unique_ptr<base::dbus::Connection> m_bus;
    std::unique_ptr<base::dbus::Object> m_object;
};

struct SoftwareUpdatePageTest : testing::Test
{
    void SetUp() override
    {
        SoftwareUpdate::setPackageKitBus(base::dbus::Connection::session());
        service = std::make_unique<FakeService>();
    }

    void TearDown() override
    {
        settingsWindow.reset();
        service.reset();
        SoftwareUpdate::setPackageKitBus(nullptr);
    }

    void openWindow()
    {
        settingsWindow = std::make_unique<SettingsWindow>();
        window().setScale(1.5);
        window().resize({900, 820});
        settingsWindow->reveal();
        window().grab();
    }

    bool openPage()
    {
        openWindow();
        if (!waitFor([&] { return inSidebar(); }))
            return false;
        settingsWindow->switchToPage("softwareupdate");
        window().grab();
        return settingsWindow->currentPage() && settingsWindow->currentPage()->title() == "Software Update";
    }

    Window& window() { return settingsWindow->window(); }
    View* view(const std::string& name) { return findView(settingsWindow->currentPage(), name); }
    std::string text(const std::string& name)
    {
        auto* label = dynamic_cast<Label*>(view(name));
        return label && isShown(label) ? label->text() : std::string();
    }
    Button* action() { return dynamic_cast<Button*>(view("softwareupdate:action")); }
    bool inSidebar()
    {
        window().grab();
        return window().layoutDump().find("\"sidebar:softwareupdate\"") != std::string::npos;
    }
    // Lets the change reach the page, and lays it out again.
    bool shows(const std::string& title)
    {
        return waitFor([&] {
            window().grab();
            return text("softwareupdate:title") == title;
        });
    }

    std::unique_ptr<FakeService> service;
    std::unique_ptr<SettingsWindow> settingsWindow;
};

} // namespace

TEST_F(SoftwareUpdatePageTest, WithoutPackageKitThereIsNoPage)
{
    openWindow();
    waitFor([] { return false; }, 200ms);
    EXPECT_FALSE(inSidebar());
}

TEST_F(SoftwareUpdatePageTest, ListsTheUpdatesToDownload)
{
    ASSERT_TRUE(service->ownPackageKit());
    ASSERT_TRUE(openPage());
    ASSERT_TRUE(shows("2 updates available"));
    EXPECT_TRUE(isShown(view("softwareupdate:list")));
    EXPECT_FALSE(isShown(view("softwareupdate:progress")));
    EXPECT_FALSE(isShown(view("softwareupdate:note")));
    ASSERT_TRUE(action());
    EXPECT_TRUE(isShown(action()));
    action()->onClicked();
    ASSERT_TRUE(waitFor([&] { return service->calls == std::vector<std::string>{"download"}; }));

    service->state = "downloading";
    service->change();
    ASSERT_TRUE(shows("Downloading updates…"));
    EXPECT_TRUE(dynamic_cast<ProgressBar*>(view("softwareupdate:progress")) != nullptr);

    service->progress = 42;
    service->change();
    ASSERT_TRUE(shows("Downloading updates… 42%"));
    auto* bar = dynamic_cast<ProgressBar*>(view("softwareupdate:progress"));
    auto* stop = dynamic_cast<HeaderButton*>(view("softwareupdate:stop"));
    ASSERT_TRUE(bar && stop);
    EXPECT_TRUE(isShown(bar));
    EXPECT_TRUE(isShown(stop));
    EXPECT_DOUBLE_EQ(bar->value(), 0.42);
    EXPECT_EQ(stop->toolTip(), "Stop");
    // The stop button ends the bar's line, centred on the bar.
    EXPECT_EQ(stop->parent(), bar->parent());
    EXPECT_NEAR(bar->geometry().y + bar->geometry().height / 2, stop->geometry().y + stop->geometry().height / 2, 0.5);
    EXPECT_GT(stop->geometry().x, bar->geometry().x + bar->geometry().width);
    EXPECT_FALSE(isShown(view("softwareupdate:detail")));
    EXPECT_FALSE(isShown(action()));
    EXPECT_TRUE(isShown(view("softwareupdate:list")));
    stop->onClicked();
    ASSERT_TRUE(waitFor([&] { return service->calls.back() == "cancel"; }));

    service->state = "ready";
    service->change();
    ASSERT_TRUE(shows("Updates are ready to install"));
    EXPECT_EQ(text("softwareupdate:detail"), "2 updates install while the computer restarts.");
    EXPECT_TRUE(isShown(view("softwareupdate:note")));
    action()->onClicked();
    // A refusal shows below the card.
    ASSERT_TRUE(waitFor([&] {
        window().grab();
        auto* note = dynamic_cast<Footnote*>(view("softwareupdate:note"));
        return note && note->label()->text().find("Not authorized") != std::string::npos;
    }));
    EXPECT_EQ(service->calls.back(), "restartAndUpdate");
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
    ASSERT_TRUE(shows("4 updates available"));
    const auto icon = [&](const std::string& package) {
        auto* view = dynamic_cast<ThemeIcon*>(this->view("softwareupdate:icon:" + package));
        return view ? view->iconName() : std::string();
    };
    ASSERT_TRUE(waitFor([&] { return icon("firefox") == "firefox"; }));
    EXPECT_EQ(icon("telegram-desktop"), "telegram");
    EXPECT_EQ(icon("claude-desktop"), "claude-desktop");
    EXPECT_EQ(icon("tzdata"), "package-x-generic");

    settingsWindow.reset();
    base::ApplicationRegistry::setInstance(loop, nullptr);
    std::filesystem::remove_all(directory);
}

TEST_F(SoftwareUpdatePageTest, ExplainsEachState)
{
    ASSERT_TRUE(service->ownPackageKit());
    service->state = "checking";
    ASSERT_TRUE(openPage());
    ASSERT_TRUE(shows("Checking for updates…"));
    EXPECT_FALSE(isShown(action()));
    EXPECT_FALSE(isShown(view("softwareupdate:list")));

    service->state = "up-to-date";
    service->updates.clear();
    service->change();
    ASSERT_TRUE(shows("The system is up to date"));
    EXPECT_FALSE(isShown(view("softwareupdate:list")));
    action()->onClicked();
    ASSERT_TRUE(waitFor([&] { return service->calls == std::vector<std::string>{"check"}; }));

    service->state = "error";
    service->error = "Cannot download the package lists";
    service->change();
    ASSERT_TRUE(shows("Cannot check for updates"));
    EXPECT_EQ(text("softwareupdate:detail"), "Cannot download the package lists");
    EXPECT_TRUE(isShown(action()));
}
