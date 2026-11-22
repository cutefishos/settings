// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <tuple>
#include <vector>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "cutefish/display/display_types.h"
#include "scenario.h"
#include "ui/keys.h"
#include "widgets/rowcombobox.h"

using namespace cutefish;
using namespace cutefish::base;
using namespace cutefish::display;
using namespace cutefish::ui;
using namespace settings;
using namespace std::chrono_literals;

namespace {

// A DRM card node that exists, for the brightness row's connector name.
std::string cardDevice()
{
    std::error_code error;
    for (const auto& entry : std::filesystem::directory_iterator("/dev/dri", error)) {
        if (entry.path().filename().string().starts_with("card"))
            return entry.path().string();
    }
    return {};
}

using Request = std::tuple<std::string, bool, RequestedMode, uint32_t, uint32_t>;

// The compositor's display service, the night light and brightness of
// cutefish-services, on a thread of their own: the page reads brightness
// with blocking calls.
class FakeServices
{
public:
    FakeServices()
    {
        std::promise<void> ready;
        m_thread = std::thread([this, &ready] { run(ready); });
        ready.get_future().get();
    }

    ~FakeServices()
    {
        m_handle.post([this] { m_loop->quit(); });
        m_thread.join();
    }

    // False when another connection owns a name: the page would talk to that
    // service instead.
    bool owned() const { return m_owned; }
    std::vector<std::vector<Request>> requests() const { return locked(m_requests); }
    std::vector<bool> previews() const { return locked(m_previews); }
    std::vector<std::string> confirmed() const { return locked(m_confirmed); }
    int32_t schedule() const { return locked(m_schedule); }
    std::vector<std::string> brightnessDisplays() const { return locked(m_brightnessDisplays); }

private:
    template<typename T>
    T locked(const T& value) const
    {
        std::lock_guard lock(m_mutex);
        return value;
    }

    void run(std::promise<void>& ready)
    {
        EventLoop loop;
        m_loop = &loop;
        m_handle = loop.handle();
        std::unique_ptr<dbus::Connection> bus = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);
        std::unique_ptr<dbus::Connection> services = dbus::Connection::open(dbus::Connection::Bus::Session, &loop);

        PhysicalDisplay panel;
        panel.sessionId = "eDP-1";
        panel.connectorName = "eDP-1";
        panel.devicePath = cardDevice();
        panel.connectionType = 1;
        panel.capabilities = ColorTransformCapability;
        panel.recommendedScale120 = 180;
        for (const auto& [width, height] : {std::pair{2160, 1440}, std::pair{1920, 1280}, std::pair{1080, 720}}) {
            Mode mode;
            mode.width = width;
            mode.height = height;
            mode.refreshMilliHz = 60000;
            mode.preferred = width == 2160;
            mode.current = width == 2160;
            panel.modes.push_back(mode);
        }
        std::vector<PhysicalDisplay> physical = {panel};
        LogicalDisplay logical;
        logical.id = "eDP-1";
        logical.destinations = {"eDP-1"};
        logical.width = 1440;
        logical.height = 960;
        logical.scale120 = 180;
        logical.primary = true;
        std::vector<LogicalDisplay> logicals = {logical};
        uint64_t revision = 1;
        std::string previewToken;

        dbus::Object display(bus.get(), "/org/cutefish/DisplayConfig1", "org.cutefish.DisplayConfig1");
        display.addMethod("GetState", [&] {
            return std::make_tuple(std::string("test"), std::string("seat0"), uint64_t(1), revision, physical,
                                   logicals, std::vector<Placement>(), previewToken,
                                   uint64_t(previewToken.empty() ? 0 : 1));
        });
        const auto configure = [&](bool preview) {
            return [&, preview](const std::string&, uint64_t, uint64_t, const std::vector<Request>& displays,
                                const std::string&, const std::vector<Placement>&, const MirrorGroups&) {
                std::string token;
                {
                    std::lock_guard lock(m_mutex);
                    m_requests.push_back(displays);
                    m_previews.push_back(preview);
                    token = "token-" + std::to_string(m_requests.size());
                }
                if (preview)
                    previewToken = token;
                ++revision;
                for (const auto& [id, enabled, mode, scale120, rotation] : displays) {
                    for (Mode& candidate : physical.front().modes)
                        candidate.current = candidate.width == mode.width && candidate.height == mode.height;
                    logicals.front().scale120 = scale120;
                    logicals.front().rotation = rotation;
                }
                display.emitSignal("OperationFinished", token, true, false, uint32_t(0), std::string(),
                                   std::vector<std::string>(), uint64_t(1), revision);
                display.emitSignal("StateChanged", uint64_t(1), revision);
                return token;
            };
        };
        display.addMethod("SetConfiguration", configure(false));
        display.addMethod("ApplyConfiguration", configure(true));
        display.addMethod("ConfirmConfiguration", [&](const std::string& token) {
            {
                std::lock_guard lock(m_mutex);
                m_confirmed.push_back(token);
            }
            previewToken.clear();
            display.emitSignal("StateChanged", uint64_t(1), revision);
        });
        display.addMethod("RevertConfiguration", [&](const std::string&) {
            previewToken.clear();
            display.emitSignal("StateChanged", uint64_t(1), revision);
        });
        display.addSignal("StateChanged", "tt");
        display.addSignal("OperationFinished", "sbbusastt");
        display.addSignal("PreviewChanged", "sttt");
        display.publish();
        m_owned = bus->requestName("org.cutefish.DisplayConfig");

        bool enabled = false;
        dbus::Object nightLight(services.get(), "/org/cutefish/Services/NightLight", "org.cutefish.Services.NightLight");
        nightLight.addProperty("enabled", [&] { return enabled; });
        nightLight.addProperty("temperature", [] { return 0.5; });
        nightLight.addProperty("schedule", [this] { return locked(m_schedule); });
        nightLight.addProperty("customStart", [] { return int32_t(18 * 60); });
        nightLight.addProperty("customEnd", [] { return int32_t(6 * 60); });
        nightLight.addProperty("available", [] { return true; });
        nightLight.addProperty("sunset", [] { return int32_t(18 * 60 + 30); });
        nightLight.addProperty("sunrise", [] { return int32_t(6 * 60 + 15); });
        nightLight.addMethod("setEnabled", [&](bool value) {
            enabled = value;
            nightLight.emitSignal("enabledChanged", value);
        });
        nightLight.addMethod("setSchedule", [&](int32_t value) {
            {
                std::lock_guard lock(m_mutex);
                m_schedule = value;
            }
            nightLight.emitSignal("scheduleChanged", value);
        });
        for (const char* name : {"enabledChanged", "availableChanged"})
            nightLight.addSignal(name, "b");
        for (const char* name :
             {"scheduleChanged", "customStartChanged", "customEndChanged", "sunsetChanged", "sunriseChanged"})
            nightLight.addSignal(name, "i");
        nightLight.addSignal("temperatureChanged", "d");
        nightLight.publish();

        if (!panel.devicePath.empty())
            m_brightnessDisplays = {std::filesystem::path(panel.devicePath).filename().string() + "-eDP-1"};
        dbus::Object brightness(services.get(), "/org/cutefish/Services/Brightness", "org.cutefish.Services.Brightness");
        brightness.addMethod("displays", [this] { return locked(m_brightnessDisplays); });
        brightness.addMethod("displayBrightness", [](const std::string&) { return int32_t(70); });
        brightness.addMethod("setDisplayBrightness", [](const std::string&, int32_t) {});
        brightness.addSignal("displaysChanged", "");
        brightness.addSignal("displayBrightnessChanged", "si");
        brightness.publish();
        m_owned = services->requestName("org.cutefish.Services") && m_owned;

        ready.set_value();
        loop.run();
    }

    std::thread m_thread;
    EventLoop* m_loop = nullptr;
    LoopHandle m_handle;
    bool m_owned = false;
    mutable std::mutex m_mutex;
    std::vector<std::vector<Request>> m_requests;
    std::vector<bool> m_previews;
    std::vector<std::string> m_confirmed;
    int32_t m_schedule = 0;
    std::vector<std::string> m_brightnessDisplays;
};

// Five seconds, as the services answer on a thread of their own.
bool waitFor(const std::function<bool()>& condition)
{
    return TestDriver::waitFor(condition, 5s);
}

struct DisplayPageTest : ::testing::Test
{
    void SetUp() override
    {
        services = std::make_unique<FakeServices>();
        ASSERT_TRUE(services->owned());
        scenario.emplace("display");
        ASSERT_TRUE(waitFor([this] { return driver().shows("display:name"); }));
    }

    void TearDown() override
    {
        scenario.reset();
        services.reset();
    }

    TestDriver& driver() { return scenario->driver; }
    std::string currentText(const std::string& name)
    {
        const auto* box = driver().find<settings::RowComboBox>(name);
        return box ? box->currentText() : std::string();
    }
    // The row of the combo box's drop-down chosen, as the user picks it.
    void choose(const std::string& name, const std::string& item)
    {
        scrollTo(driver(), "page:display", name);
        driver().click(name);
        driver().choosePopupItem(item);
    }

    std::unique_ptr<FakeServices> services;
    std::optional<Scenario> scenario;
};

} // namespace

TEST_F(DisplayPageTest, NamesTheOnlyScreen)
{
    EXPECT_EQ(driver().texts("display:name"), std::vector<std::string>{"Built-in Display"});
    EXPECT_FALSE(driver().shows("display:tabs"));
    EXPECT_FALSE(driver().shows("display:arrangement"));
    EXPECT_FALSE(driver().shows("display:error"));
    EXPECT_EQ(currentText("display:resolution"), "2160 × 1440 (Recommended)");
    const auto* resolution = driver().find<settings::RowComboBox>("display:resolution");
    ASSERT_NE(resolution, nullptr);
    EXPECT_EQ(resolution->count(), 3);
    EXPECT_EQ(currentText("display:refresh"), "60.00 Hz");
    EXPECT_EQ(currentText("display:scale"), "150% (Recommended)");
    EXPECT_EQ(currentText("display:rotation"), "Normal");
    EXPECT_EQ(driver().shows("display:brightness"), !services->brightnessDisplays().empty());
}

TEST_F(DisplayPageTest, ScaleAppliesAtOnce)
{
    choose("display:scale", "125%");
    ASSERT_TRUE(waitFor([this] { return services->requests().size() == 1; }));
    EXPECT_FALSE(services->previews().front());
    EXPECT_EQ(std::get<3>(services->requests().front().front()), 150u);
    ASSERT_TRUE(waitFor([this] { return currentText("display:scale") == "125%"; }));
}

TEST_F(DisplayPageTest, ResolutionWaitsForConfirmation)
{
    choose("display:resolution", "1920 × 1280");
    ASSERT_TRUE(waitFor([this] { return services->requests().size() == 1; }));
    EXPECT_TRUE(services->previews().front());
    EXPECT_EQ(std::get<2>(services->requests().front().front()).width, 1920);
    // The confirmation opens over the window; Return keeps the mode.
    ASSERT_TRUE(waitFor([this] { return driver().dialog() != nullptr; }));
    auto confirmation = scenario->dialog();
    ASSERT_TRUE(confirmation);
    confirmation->typeKey(key::Return);
    ASSERT_TRUE(waitFor([this] { return services->confirmed().size() == 1; }));
    EXPECT_EQ(services->confirmed().front(), "token-1");
    confirmation.reset();
    ASSERT_TRUE(waitFor([this] { return driver().dialog() == nullptr; }));
    EXPECT_EQ(currentText("display:resolution"), "1920 × 1280");
}

TEST_F(DisplayPageTest, NightLightScheduleRows)
{
    ASSERT_TRUE(waitFor([this] { return driver().shows("display:nightlight"); }));
    EXPECT_FALSE(driver().shows("display:sunset"));
    EXPECT_FALSE(driver().shows("display:from"));
    choose("display:schedule", "Sunset to sunrise");
    ASSERT_TRUE(waitFor([this] { return services->schedule() == 1 && driver().shows("display:sunset"); }));
    // Sunset 18:30 in the session's short time format.
    const std::vector<std::string> sunset = driver().texts("display:sunset");
    EXPECT_TRUE(std::any_of(sunset.begin(), sunset.end(),
                            [](const std::string& text) { return text.find("30") != std::string::npos; }));
    choose("display:schedule", "Custom");
    ASSERT_TRUE(waitFor([this] { return driver().shows("display:from") && !driver().shows("display:sunset"); }));
}
