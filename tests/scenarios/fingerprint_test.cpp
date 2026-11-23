// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "fprint_fake.h"
#include "pages/fingerprint/fingerprintview.h"
#include "pages/fingerprint/fingers.h"
#include "scenario.h"
#include "service_thread.h"

using namespace cutefish::base;
using namespace cutefish::ui;
using namespace settings;
using settings::testing::FakeFprintd;
using settings::testing::ServiceThread;

namespace {

struct FingerprintTest : ::testing::Test
{
    void SetUp() override
    {
        if (!settings::testing::hasSessionBus())
            GTEST_SKIP() << "no session bus";
        setFingerprintBus(dbus::Connection::session());
    }

    void TearDown() override
    {
        scenario.reset();
        if (service) {
            service.reset();
            settings::testing::waitForNameGone("net.reactivated.Fprint");
        }
        setFingerprintBus(nullptr);
    }

    void start(std::vector<std::string> enrolled)
    {
        fake.enrolled = std::move(enrolled);
        service.emplace("net.reactivated.Fprint",
                        [this](dbus::Connection& bus) { return settings::testing::serveFprintd(bus, fake); });
        ASSERT_TRUE(service->start());
    }

    // The page once fprintd has told what it holds.
    TestDriver& open()
    {
        scenario = std::make_unique<Scenario>("fingerprint");
        TestDriver::waitFor([this] { return driver().shows("fingerprint:list") || driver().shows("fingerprint:empty"); });
        driver().layout();
        return driver();
    }

    TestDriver& driver() { return scenario->driver; }

    void status(const std::string& result, bool done)
    {
        service->post([this, result, done] { fake.status(result, done); });
    }

    // Clicks Add Fingerprint and waits for the dialog, which opens once the
    // sensor has been claimed and the enrollment started.
    std::unique_ptr<TestDriver> addFingerprint()
    {
        EXPECT_TRUE(driver().click("fingerprint:add"));
        EXPECT_TRUE(TestDriver::waitFor([this] { return driver().dialog() != nullptr; }));
        return scenario->dialog();
    }

    FakeFprintd fake;
    std::optional<ServiceThread> service;
    std::unique_ptr<Scenario> scenario;
};

} // namespace

TEST_F(FingerprintTest, ListsTheStoredFingerprintsInOrder)
{
    start({"left-thumb", "right-index-finger"});
    TestDriver& page = open();
    EXPECT_TRUE(page.shows("fingerprint:list"));
    EXPECT_FALSE(page.shows("fingerprint:empty"));
    EXPECT_EQ(page.texts("fingerprint:row:1"), (std::vector<std::string>{"Fingerprint 1", "Delete"}));
    EXPECT_EQ(page.texts("fingerprint:row:2"), (std::vector<std::string>{"Fingerprint 2", "Delete"}));
    EXPECT_FALSE(page.shows("fingerprint:row:3"));
    EXPECT_TRUE(page.shows("fingerprint:add"));
}

TEST_F(FingerprintTest, WithoutFingerprintsThePageInvitesToAddOne)
{
    start({});
    TestDriver& page = open();
    EXPECT_TRUE(page.shows("fingerprint:empty"));
    EXPECT_FALSE(page.shows("fingerprint:list"));
    EXPECT_TRUE(page.shows("fingerprint:add"));
}

TEST_F(FingerprintTest, FiveFingerprintsAreTheMost)
{
    start({"right-index-finger", "left-index-finger", "right-middle-finger", "left-middle-finger", "right-thumb"});
    TestDriver& page = open();
    EXPECT_TRUE(page.shows("fingerprint:row:5"));
    EXPECT_FALSE(page.shows("fingerprint:add"));
}

TEST_F(FingerprintTest, AddingAFingerprintFillsInThePrintStepByStep)
{
    start({"right-index-finger"});
    open();
    auto dialog = addFingerprint();
    ASSERT_TRUE(dialog);
    // The page waits behind the dialog.
    EXPECT_FALSE(driver().find("fingerprint:add")->isEnabled());

    EXPECT_EQ(dialog->text("enroll:title"), "Place your finger on the sensor");
    // The close control is the only way out while scanning.
    EXPECT_TRUE(dialog->findAll("enroll:cancel").empty());
    EXPECT_FALSE(dialog->shows("enroll:action"));
    auto* print = dialog->find<FingerprintView>("enroll:print");
    ASSERT_NE(print, nullptr);
    EXPECT_EQ(print->progress(), 0);

    // Six stages: two thirds are four.
    for (int i = 0; i < 3; ++i)
        status("enroll-stage-passed", false);
    ASSERT_TRUE(TestDriver::waitFor([&] { return print->progress() == 0.5; }));
    EXPECT_EQ(dialog->text("enroll:title"), "Place your finger on the sensor");

    status("enroll-retry-scan", false);
    ASSERT_TRUE(TestDriver::waitFor([&] { return dialog->text("enroll:detail").starts_with("The sensor could not read"); }));

    status("enroll-stage-passed", false);
    ASSERT_TRUE(TestDriver::waitFor([&] { return dialog->text("enroll:title") == "Adjust your grip"; }));
    EXPECT_NEAR(print->progress(), 4.0 / 6, 1e-9);

    status("enroll-stage-passed", false);
    status("enroll-stage-passed", false);
    status("enroll-completed", true);
    ASSERT_TRUE(TestDriver::waitFor([&] { return dialog->text("enroll:title") == "Fingerprint added"; }));
    EXPECT_EQ(print->progress(), 1);
    ASSERT_TRUE(dialog->shows("enroll:action"));
    EXPECT_EQ(dialog->text("enroll:action"), "Done");

    dialog->click("enroll:action");
    ASSERT_TRUE(TestDriver::waitFor([&] { return driver().dialog() == nullptr; }));
    // The new print is on the page, and the sensor was given back.
    ASSERT_TRUE(TestDriver::waitFor([&] { return driver().shows("fingerprint:row:2"); }));
    EXPECT_TRUE(driver().find("fingerprint:add")->isEnabled());
    EXPECT_EQ(fake.storedFingers().size(), 2u);
    EXPECT_TRUE(TestDriver::waitFor([&] { return fake.releases == 1; }));
}

TEST_F(FingerprintTest, ClosingTheDialogStopsTheEnrollment)
{
    start({});
    open();
    auto dialog = addFingerprint();
    ASSERT_TRUE(dialog);
    status("enroll-stage-passed", false);
    dialog->typeKey(key::Escape);
    ASSERT_TRUE(TestDriver::waitFor([&] { return driver().dialog() == nullptr; }));
    ASSERT_TRUE(TestDriver::waitFor([&] { return fake.releases == 1; }));
    EXPECT_EQ(fake.enrollStops, 1);
    EXPECT_TRUE(fake.storedFingers().empty());
    EXPECT_TRUE(driver().find("fingerprint:add")->isEnabled());
}

TEST_F(FingerprintTest, RefusingThePasswordOpensNoDialog)
{
    fake.enrollError = "net.reactivated.Fprint.Error.PermissionDenied";
    start({});
    open();
    ASSERT_TRUE(driver().click("fingerprint:add"));
    ASSERT_TRUE(TestDriver::waitFor([&] { return fake.releases == 1; }));
    ASSERT_TRUE(TestDriver::waitFor([&] { return driver().find("fingerprint:add")->isEnabled(); }));
    EXPECT_EQ(driver().dialog(), nullptr);
}

TEST_F(FingerprintTest, ADuplicateCanBeTriedAgain)
{
    start({});
    open();
    auto dialog = addFingerprint();
    ASSERT_TRUE(dialog);
    status("enroll-duplicate", true);
    ASSERT_TRUE(TestDriver::waitFor([&] { return dialog->text("enroll:title") == "This fingerprint is already added"; }));
    ASSERT_TRUE(dialog->shows("enroll:action"));
    EXPECT_EQ(dialog->text("enroll:action"), "Try Again");

    dialog->click("enroll:action");
    ASSERT_TRUE(TestDriver::waitFor([&] { return fake.claims == 2; }));
    ASSERT_TRUE(TestDriver::waitFor([&] { return dialog->text("enroll:title") == "Place your finger on the sensor"; }));
    EXPECT_FALSE(dialog->shows("enroll:action"));
}

TEST_F(FingerprintTest, ABusySensorIsSaidSo)
{
    fake.claimError = "net.reactivated.Fprint.Error.AlreadyInUse";
    start({});
    open();
    auto dialog = addFingerprint();
    ASSERT_TRUE(dialog);
    EXPECT_EQ(dialog->text("enroll:title"), "The sensor is busy");
    EXPECT_TRUE(dialog->shows("enroll:action"));
}

TEST_F(FingerprintTest, DeletingAsksFirst)
{
    start({"right-index-finger", "left-index-finger"});
    open();
    ASSERT_TRUE(driver().click("fingerprint:delete:1"));
    {
        Window* alert = driver().dialog();
        ASSERT_NE(alert, nullptr);
        TestDriver alertDriver(*alert, scenarioFailure);
        alertDriver.setKeyboardFocus(true);
        EXPECT_TRUE(alertDriver.showsText("Delete this fingerprint"));
        ASSERT_TRUE(alertDriver.clickText("Cancel"));
    }
    ASSERT_TRUE(TestDriver::waitFor([&] { return driver().dialog() == nullptr; }));
    EXPECT_EQ(fake.storedFingers().size(), 2u);

    ASSERT_TRUE(driver().click("fingerprint:delete:1"));
    {
        Window* alert = driver().dialog();
        ASSERT_NE(alert, nullptr);
        TestDriver alertDriver(*alert, scenarioFailure);
        alertDriver.setKeyboardFocus(true);
        ASSERT_TRUE(alertDriver.clickText("Delete"));
    }
    ASSERT_TRUE(TestDriver::waitFor([&] { return !driver().shows("fingerprint:row:2"); }));
    EXPECT_EQ(fake.storedFingers(), (std::vector<std::string>{"left-index-finger"}));
    EXPECT_TRUE(driver().shows("fingerprint:row:1"));
    EXPECT_TRUE(TestDriver::waitFor([&] { return fake.releases == 1; }));
}

TEST_F(FingerprintTest, ThePrintIsCentredInTheDialog)
{
    start({});
    open();
    auto dialog = addFingerprint();
    ASSERT_TRUE(dialog);
    dialog->layout();
    const double width = dialog->window().size().width;
    for (const char* name : {"enroll:print", "enroll:title", "enroll:detail"}) {
        const RectF frame = dialog->frame(name);
        EXPECT_NEAR(frame.x + frame.width / 2, width / 2, 0.5) << name;
    }
}

TEST_F(FingerprintTest, TheButtonTurningUpMovesNothing)
{
    start({});
    open();
    auto dialog = addFingerprint();
    ASSERT_TRUE(dialog);
    dialog->layout();
    const RectF print = dialog->frame("enroll:print");
    const RectF title = dialog->frame("enroll:title");

    for (int i = 0; i < 5; ++i)
        status("enroll-stage-passed", false);
    // The longer guidance of the last step has two lines.
    ASSERT_TRUE(TestDriver::waitFor([&] { return dialog->text("enroll:title") == "Adjust your grip"; }));
    dialog->layout();
    EXPECT_EQ(dialog->frame("enroll:print").y, print.y);
    EXPECT_EQ(dialog->frame("enroll:title").y, title.y);

    status("enroll-completed", true);
    ASSERT_TRUE(TestDriver::waitFor([&] { return dialog->shows("enroll:action"); }));
    dialog->layout();
    EXPECT_EQ(dialog->frame("enroll:print").y, print.y);
    EXPECT_EQ(dialog->frame("enroll:title").y, title.y);
    // The button spans the dialog's content and sits at its bottom.
    const RectF action = dialog->frame("enroll:action");
    EXPECT_GT(action.width, 250);
    EXPECT_NEAR(action.x + action.width / 2, dialog->window().size().width / 2, 0.5);
}
