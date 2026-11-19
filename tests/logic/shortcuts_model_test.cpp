// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

// ShortcutsModel against a stand-in for the compositor's
// org.cutefish.Shortcuts1 service.

#include <gtest/gtest.h>

#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "pages/shortcuts/shortcutsmodel.h"
#include "service_thread.h"
#include "shortcuts_fake.h"

using namespace cutefish::base;
using namespace settings;
using settings::testing::FakeShortcuts;
using settings::testing::ServiceThread;
using settings::testing::accelerator;
using settings::testing::waitFor;

namespace {

struct ShortcutsModelTest : ::testing::Test
{
    void SetUp() override
    {
        if (!settings::testing::hasSessionBus())
            GTEST_SKIP() << "no session bus";
        service.emplace("org.cutefish.Shortcuts1",
                        [this](dbus::Connection& bus) { return settings::testing::serveShortcuts(bus, fake); });
    }

    void TearDown() override
    {
        if (!service)
            return;
        service.reset();
        settings::testing::waitForNameGone("org.cutefish.Shortcuts1");
    }

    FakeShortcuts fake;
    std::optional<ServiceThread> service;
};

} // namespace

TEST_F(ShortcutsModelTest, ListsKnownActionsInLayoutOrderWithReadableKeys)
{
    ASSERT_TRUE(service->start());
    ShortcutsModel model;
    ASSERT_TRUE(waitFor([&] { return model.isAvailable() && model.actions().size() == 2; }));
    // Layout order, not the service's order: lock-screen is in General after
    // the launchpad entries, toggle-maximized is in Windows.
    EXPECT_EQ(model.actions()[0].name, "lock-screen");
    EXPECT_EQ(model.actions()[1].name, "toggle-maximized");
    EXPECT_EQ(model.actions()[1].category, "Windows");
    EXPECT_TRUE(model.actions()[1].isDefault());

    const std::vector<ShortcutsModel::Accelerator>& accelerators = model.actions()[1].accelerators;
    ASSERT_EQ(accelerators.size(), 2u);
    EXPECT_EQ(accelerators[0].parts, (std::vector<std::string>{"Alt", "="}));
    EXPECT_EQ(accelerators[1].parts, (std::vector<std::string>{"Meta", "↑"}));
    EXPECT_EQ(accelerators[1].text, "Meta + ↑");
    EXPECT_FALSE(model.hasCustomAccelerators());

    // A change on the service side reloads the model.
    {
        std::lock_guard guard(fake.mutex);
        fake.maximize = "[" + accelerator(0x6b, "k", "K", 8, "user") + "]";
    }
    service->post([this] { fake.object->emitSignal("acceleratorsChanged"); });
    ASSERT_TRUE(waitFor([&] { return model.hasCustomAccelerators(); }));
    EXPECT_FALSE(model.actions()[1].isDefault());

    // Requests reach the service with the compositor's modifier bits, and
    // results come back as messages the dialog shows.
    ShortcutsModel::Result result = model.addAccelerator("toggle-maximized", 16, 0, ShortcutsModel::MetaDown);
    EXPECT_EQ(result.result, "success");
    EXPECT_TRUE(result.message.empty());
    {
        std::lock_guard guard(fake.mutex);
        fake.nextResult = "{\"result\":\"conflict-can-override\",\"conflictAction\":\"lock-screen\"}";
    }
    result = model.replaceAccelerator("toggle-maximized", 0x6b, 8, 38, 0, 8);
    EXPECT_EQ(result.result, "conflict-can-override");
    EXPECT_EQ(result.conflictDescription, "Lock the screen");
    EXPECT_NE(result.message.find("Lock the screen"), std::string::npos);
    model.removeAccelerator("toggle-maximized", 0x6b, 8);
    model.restoreDefault("toggle-maximized");
    model.restoreAllDefaults();
    model.setRecording(true);
    EXPECT_EQ(model.partsForKey(16, 'q', ShortcutsModel::ControlDown | ShortcutsModel::ShiftDown),
              (std::vector<std::string>{"Ctrl", "Shift", "Q"}));
    model.setRecording(false);
    ASSERT_TRUE(waitFor([&] { return fake.takeCalls().size() == 8; }));
    const std::vector<std::string> calls = fake.takeCalls();
    EXPECT_EQ(calls[0], "add toggle-maximized 16 0 8");
    EXPECT_EQ(calls[1], "replace toggle-maximized 107 8 38 0 8");
    EXPECT_EQ(calls[2], "remove toggle-maximized 107 8");
    EXPECT_EQ(calls[3], "restore toggle-maximized");
    EXPECT_EQ(calls[4], "restore-all");
    EXPECT_EQ(calls[5], "prevent 1");
    EXPECT_EQ(calls[6], "describe 16 81");
    EXPECT_EQ(calls[7], "prevent 0");

    EXPECT_EQ(ShortcutsModel::serviceKeysym(0xffc1), 0xffc1u);
    EXPECT_EQ(ShortcutsModel::serviceKeysym(0xff52), 0xff52u);
    EXPECT_EQ(ShortcutsModel::serviceKeysym('q'), uint32_t('Q'));
    EXPECT_TRUE(ShortcutsModel::isModifierKey(0xffeb));
    EXPECT_FALSE(ShortcutsModel::isModifierKey('q'));

    service->stop();
    ASSERT_TRUE(waitFor([&] { return !model.isAvailable(); }));
    EXPECT_TRUE(model.actions().empty());
}

TEST_F(ShortcutsModelTest, TheCommandConventionNamesTheKeys)
{
    fake.convention = "command";
    ASSERT_TRUE(service->start());
    ShortcutsModel model;
    ASSERT_TRUE(waitFor([&] { return model.isAvailable() && model.commandKeys(); }));
    EXPECT_EQ(model.modifierParts(ShortcutsModel::ControlDown | ShortcutsModel::AltDown | ShortcutsModel::MetaDown),
              (std::vector<std::string>{"Control", "Option", "Command"}));
}

TEST_F(ShortcutsModelTest, ReportsTheServiceMissing)
{
    ShortcutsModel model;
    EXPECT_FALSE(model.isAvailable());
    EXPECT_TRUE(model.actions().empty());
    const ShortcutsModel::Result result = model.restoreAllDefaults();
    EXPECT_EQ(result.result, "unavailable");
    EXPECT_FALSE(result.message.empty());
}
