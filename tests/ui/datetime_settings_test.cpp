// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <atomic>
#include <cstdio>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/file.h"
#include "cutefish/timedate/time_date.h"
#include "input_test_support.h"
#include "pages/datetime/time.h"

using namespace cutefish;
using namespace cutefish::base;
using settings::testing::ServiceThread;
using settings::testing::waitFor;

namespace {

// timedated and cutefish-services on a private session bus: timedated
// refuses to turn network time off, as a cancelled password does.
struct TimeSettingsFixture : ::testing::Test
{
    void SetUp() override
    {
        dbus::Connection* bus = dbus::Connection::session();
        const dbus::Proxy daemon(bus, "org.freedesktop.DBus", "/org/freedesktop/DBus", "org.freedesktop.DBus");
        if (!bus || daemon.call("NameHasOwner", std::string("org.cutefish.Services")).value<bool>().value_or(true))
            GTEST_SKIP() << "Needs a private session bus: run the tests in dbus-run-session";

        timedated = std::make_unique<ServiceThread>("org.freedesktop.timedate1", [this](dbus::Connection& bus) {
            auto object = std::make_shared<dbus::Object>(&bus, "/org/freedesktop/timedate1", "org.freedesktop.timedate1");
            object->addProperty("NTP", [this] { return ntp.load(); });
            object->addProperty("CanNTP", [] { return true; });
            object->addProperty("Timezone", [] { return std::string("Asia/Singapore"); });
            object->addMethod("SetNTP", "bb", "", [this, object = object.get()](dbus::MethodCall& call) {
                bool enabled = false, interactive = false;
                call.read(enabled, interactive);
                if (!enabled) {
                    call.replyError("org.freedesktop.DBus.Error.AccessDenied", "Cancelled");
                    return;
                }
                ntp = enabled;
                object->emitPropertiesChanged({"NTP"});
                call.reply();
            });
            object->publish();
            return std::shared_ptr<void>(object);
        });
        services = std::make_unique<ServiceThread>("org.cutefish.Services", [this](dbus::Connection& bus) {
            auto object = std::make_shared<dbus::Object>(&bus, "/org/cutefish/Services/Language",
                                                         "org.cutefish.Services.Language");
            object->addMethod("setTwentyFourHour", [this](int32_t value) { twentyFour = value; });
            object->publish();
            return std::shared_ptr<void>(object);
        });
        ASSERT_TRUE(timedated->start());
        ASSERT_TRUE(services->start());
        ASSERT_TRUE(directory.isValid());
        configPath = directory.path() + "/services.conf";
        std::FILE* file = std::fopen(configPath.c_str(), "w");
        ASSERT_NE(file, nullptr);
        std::fputs("[Language]\nlanguage=en_US\n\n[Region]\nregion=US\n", file);
        std::fclose(file);
    }

    void TearDown() override
    {
        // The next test checks the bus is private by the services' name.
        if (services) {
            services->stop();
            timedated->stop();
            settings::testing::waitForNameGone("org.cutefish.Services");
            settings::testing::waitForNameGone("org.freedesktop.timedate1");
        }
    }

    file::TemporaryDirectory directory {"/tmp", "datetime-settings"};
    std::string configPath;
    std::unique_ptr<ServiceThread> timedated;
    std::unique_ptr<ServiceThread> services;
    std::atomic<bool> ntp = true;
    std::atomic<int> twentyFour = -1;
};

} // namespace

TEST_F(TimeSettingsFixture, ARefusedChangeOfNetworkTimeGoesBack)
{
    timedate::TimeDate timeDate(dbus::Connection::session());
    settings::TimeSettings time(timeDate, configPath);
    ASSERT_TRUE(time.canNtp());
    ASSERT_TRUE(time.useNtp());
    std::vector<bool> shown;
    Subscription subscription = time.useNtpChanged.connect([&] { shown.push_back(time.useNtp()); });
    time.setUseNtp(false);
    // At once, then back once timedated refused.
    EXPECT_FALSE(time.useNtp());
    ASSERT_TRUE(waitFor([&] { return time.useNtp(); }));
    EXPECT_EQ(shown, (std::vector<bool>{false, true}));
}

TEST_F(TimeSettingsFixture, TheTwentyFourHourClockGoesToTheServices)
{
    timedate::TimeDate timeDate(dbus::Connection::session());
    settings::TimeSettings time(timeDate, configPath);
    // The region's 12-hour clock until a choice is made.
    EXPECT_FALSE(time.twentyFour());
    time.setTwentyFour(true);
    EXPECT_TRUE(time.twentyFour());
    ASSERT_TRUE(waitFor([&] { return twentyFour != -1; }));
    EXPECT_EQ(twentyFour.load(), 2);
}
