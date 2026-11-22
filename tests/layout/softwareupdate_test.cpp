// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/timer.h"
#include "pages/softwareupdate/softwareupdate.h"
#include "softwareupdate_fake.h"
#include "ui/application.h"
#include "ui/header_button.h"
#include "ui/label.h"
#include "ui/progress_bar.h"
#include "ui/window.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;
using namespace settings;
using namespace std::chrono_literals;
using settings::testing::FakeService;

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

struct SoftwareUpdatePageTest : ::testing::Test
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

TEST_F(SoftwareUpdatePageTest, TheButtonsCentreOnTheirLines)
{
    ASSERT_TRUE(service->ownPackageKit());
    ASSERT_TRUE(openPage());
    ASSERT_TRUE(shows("2 updates available"));
    // The check button follows the list title's text, centred on its line.
    auto* check = view("softwareupdate:check");
    ASSERT_TRUE(check);
    const auto labels = findAll<SectionLabel>(view("softwareupdate:list"));
    ASSERT_FALSE(labels.empty());
    Label* title = labels.front()->label();
    const PointF titleOrigin = title->mapToWindow({0, 0});
    const PointF checkOrigin = check->mapToWindow({0, 0});
    EXPECT_NEAR(titleOrigin.y + title->geometry().height / 2, checkOrigin.y + check->geometry().height / 2, 0.5);
    EXPECT_GT(checkOrigin.x, titleOrigin.x + title->geometry().width);

    service->state = "updating";
    service->progress = 42;
    service->canCancel = true;
    service->change();
    ASSERT_TRUE(shows("Downloading updates… 42%"));
    auto* bar = dynamic_cast<ProgressBar*>(view("softwareupdate:progress"));
    auto* stop = dynamic_cast<HeaderButton*>(view("softwareupdate:stop"));
    ASSERT_TRUE(bar && stop);
    // The stop button ends the bar's line, centred on the bar.
    const RectF barRect = {bar->mapToWindow({0, 0}).x, bar->mapToWindow({0, 0}).y, bar->geometry().width,
                           bar->geometry().height};
    const PointF stopOrigin = stop->mapToWindow({0, 0});
    EXPECT_NEAR(barRect.y + barRect.height / 2, stopOrigin.y + stop->geometry().height / 2, 0.5);
    EXPECT_GT(stopOrigin.x, barRect.x + barRect.width);

    // Without the stop button the bar keeps its length.
    service->phase = "installing";
    service->progress = 80;
    service->canCancel = false;
    service->change();
    ASSERT_TRUE(shows("Installing updates… 80%"));
    EXPECT_DOUBLE_EQ(bar->mapToWindow({0, 0}).x, barRect.x);
    EXPECT_DOUBLE_EQ(bar->geometry().width, barRect.width);
}
