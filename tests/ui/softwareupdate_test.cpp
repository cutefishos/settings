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
#include "ui/dialog_window.h"
#include "ui/keys.h"
#include "ui/test_input.h"
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
        m_object->addProperty("phase", [this] { return phase; });
        m_object->addProperty("progress", [this] { return progress; });
        m_object->addProperty("canCancel", [this] { return canCancel; });
        m_object->addProperty("error", [this] { return error; });
        m_object->addMethod("check", [this] { calls.push_back("check"); });
        m_object->addMethod("update", [this] { calls.push_back("update"); });
        m_object->addMethod("cancel", [this] { calls.push_back("cancel"); });
        m_object->addMethod("restart", [this] { calls.push_back("restart"); });
        EXPECT_TRUE(m_object->publish());
        EXPECT_TRUE(m_bus->requestName("org.cutefish.Services"));
    }

    bool ownPackageKit() { return m_bus->requestName("org.freedesktop.PackageKit"); }

    void change()
    {
        m_object->emitPropertiesChanged({"state", "updates", "phase", "progress", "canCancel", "error"});
    }

    std::string state = "available";
    std::vector<UpdateEntry> updates = {
        {"openssl", "3.5.4-1~deb13u2", "Secure Sockets Layer toolkit"},
        {"tzdata", "2026f-0+deb13u1", "time zone and daylight-saving time data"},
    };
    std::string phase = "downloading";
    int32_t progress = -1;
    bool canCancel = false;
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
    std::string actionText()
    {
        const auto labels = findAll<Label>(action());
        return labels.empty() ? std::string() : labels.front()->text();
    }
    // The shown confirmation over the window, if any.
    Window* confirmation()
    {
        for (Window* window : Application::instance()->windows()) {
            if (window != &this->window() && window->contentView() && !window->isHidden()
                && !findAll<DialogButton>(window->contentView()).empty())
                return window;
        }
        return nullptr;
    }
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

TEST_F(SoftwareUpdatePageTest, InstallsAndSuggestsARestart)
{
    ASSERT_TRUE(service->ownPackageKit());
    ASSERT_TRUE(openPage());
    ASSERT_TRUE(shows("2 updates available"));
    EXPECT_TRUE(isShown(view("softwareupdate:list")));
    EXPECT_FALSE(isShown(view("softwareupdate:progress")));
    ASSERT_TRUE(action());
    EXPECT_TRUE(isShown(action()));
    EXPECT_EQ(actionText(), "Update Now");
    action()->onClicked();
    ASSERT_TRUE(waitFor([&] { return service->calls == std::vector<std::string>{"update"}; }));

    service->state = "updating";
    service->progress = 42;
    service->canCancel = true;
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
    const RectF barRect = {bar->mapToWindow({0, 0}).x, bar->mapToWindow({0, 0}).y, bar->geometry().width,
                           bar->geometry().height};
    const PointF stopOrigin = stop->mapToWindow({0, 0});
    EXPECT_NEAR(barRect.y + barRect.height / 2, stopOrigin.y + stop->geometry().height / 2, 0.5);
    EXPECT_GT(stopOrigin.x, barRect.x + barRect.width);
    EXPECT_FALSE(isShown(view("softwareupdate:detail")));
    EXPECT_FALSE(isShown(action()));
    EXPECT_TRUE(isShown(view("softwareupdate:list")));
    stop->onClicked();
    ASSERT_TRUE(waitFor([&] { return service->calls.back() == "cancel"; }));

    // Installing can no longer be stopped; the bar keeps its length.
    service->phase = "installing";
    service->progress = 80;
    service->canCancel = false;
    service->change();
    ASSERT_TRUE(shows("Installing updates… 80%"));
    EXPECT_FALSE(isShown(stop));
    EXPECT_TRUE(isShown(bar));
    EXPECT_DOUBLE_EQ(bar->mapToWindow({0, 0}).x, barRect.x);
    EXPECT_DOUBLE_EQ(bar->geometry().width, barRect.width);

    service->state = "restart";
    service->updates.clear();
    service->change();
    ASSERT_TRUE(shows("Updates installed"));
    EXPECT_EQ(text("softwareupdate:detail"), "Restart the computer to finish installing them.");
    EXPECT_FALSE(isShown(view("softwareupdate:list")));
    EXPECT_EQ(actionText(), "Restart");

    // Nothing restarts before the confirmation, and not when it is turned down.
    action()->onClicked();
    Window* dialog = confirmation();
    ASSERT_TRUE(dialog);
    waitFor([] { return false; }, 100ms);
    EXPECT_EQ(std::count(service->calls.begin(), service->calls.end(), "restart"), 0);
    TestInput::setKeyboardFocus(*dialog, true);
    TestInput::typeKey(*dialog, key::Escape);
    EXPECT_TRUE(waitFor([&] { return confirmation() == nullptr; }));
    waitFor([] { return false; }, 100ms);
    EXPECT_EQ(std::count(service->calls.begin(), service->calls.end(), "restart"), 0);

    action()->onClicked();
    dialog = confirmation();
    ASSERT_TRUE(dialog);
    dialog->grab();
    const auto buttons = findAll<DialogButton>(dialog->contentView());
    ASSERT_EQ(buttons.size(), 2u);
    const RectF restart = buttons[1]->geometry();
    const PointF origin = buttons[1]->mapToWindow({0, 0});
    TestInput::click(*dialog, {origin.x + restart.width / 2, origin.y + restart.height / 2});
    ASSERT_TRUE(waitFor([&] { return std::count(service->calls.begin(), service->calls.end(), "restart") == 1; }));
    waitFor([] { return false; }, 100ms);
    EXPECT_EQ(std::count(service->calls.begin(), service->calls.end(), "restart"), 1);
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

    // A failure while updating says so.
    service->state = "updating";
    service->change();
    ASSERT_TRUE(shows("Downloading updates…"));
    service->state = "error";
    service->error = "dpkg returned an error code (1)";
    service->change();
    ASSERT_TRUE(shows("The updates could not be installed"));
}
