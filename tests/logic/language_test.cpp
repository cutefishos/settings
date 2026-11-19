// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/locale.h"
#include "base/settings.h"
#include "language_config.h"
#include "pages/language/language.h"
#include "pages/language/region.h"
#include "service_thread.h"

using namespace cutefish;
using namespace settings;
using settings::testing::LanguageConfig;
using namespace std::chrono_literals;

namespace {

// Five seconds, as the catalog loads on a thread.
bool waitFor(const std::function<bool()>& condition)
{
    return settings::testing::waitFor(condition, 5s);
}

std::vector<std::string> codes(const std::vector<LanguageEntry>& entries)
{
    std::vector<std::string> result;
    for (const LanguageEntry& entry : entries)
        result.push_back(entry.code);
    return result;
}

} // namespace

TEST(Language, OpeningThePageDoesNotLoadTheCatalog)
{
    LanguageConfig config({"en_US", "zh_Hans_CN"});
    Language language(config.path);
    EXPECT_EQ(codes(language.preferredLanguages()), (std::vector<std::string>{"en_US", "zh_Hans_CN"}));
    EXPECT_TRUE(language.availableLanguages().empty());
    EXPECT_FALSE(language.isLoading());
}

TEST(Language, CatalogAndSearch)
{
    LanguageConfig config({"en_US", "zh_Hans_CN"});
    Language language(config.path);
    language.loadLanguages();
    ASSERT_TRUE(waitFor([&] { return !language.isLoading() && !language.availableLanguages().empty(); }));
    std::vector<std::string> available = codes(language.availableLanguages());
    ASSERT_EQ(available.size(), 40u);
    EXPECT_EQ(available.front(), "en_GB");
    EXPECT_EQ(available[available.size() - 2], "zh_Hant_HK");
    EXPECT_EQ(available.back(), "zh_Hant_TW");

    // Preferred languages are left out even when searched for.
    language.search("zh_Hans_CN");
    EXPECT_TRUE(language.availableLanguages().empty());
    // A language's own name, its code, ignoring case and the space around it.
    language.search("Deutsch");
    EXPECT_EQ(codes(language.availableLanguages()), std::vector<std::string>{"de_DE"});
    language.search("it_IT");
    EXPECT_EQ(language.availableLanguages().size(), 1u);
    language.search("no-such-language");
    EXPECT_TRUE(language.availableLanguages().empty());
    language.search("  ID_id  ");
    EXPECT_EQ(codes(language.availableLanguages()), std::vector<std::string>{"id_ID"});
    language.search("");
    EXPECT_EQ(language.availableLanguages().size(), 40u);
}

TEST(Language, ClosingWhileLoadingLeavesTheCatalogForTheNextPage)
{
    LanguageConfig config({"en_US", "zh_Hans_CN"});
    auto language = std::make_unique<Language>(config.path);
    language->loadLanguages();
    language.reset();
    Language reopened(config.path);
    reopened.loadLanguages();
    ASSERT_TRUE(waitFor([&] { return !reopened.isLoading() && reopened.availableLanguages().size() == 40; }));
}

TEST(Region, CatalogNamesEveryRegionInOrder)
{
    const std::vector<RegionEntry> regions = Region::catalog("en_US");
    ASSERT_GT(regions.size(), 240u);
    const auto find = [&regions](const std::string& code) {
        return std::find_if(regions.begin(), regions.end(), [&code](const RegionEntry& entry) { return entry.code == code; });
    };
    ASSERT_NE(find("SG"), regions.end());
    EXPECT_EQ(find("SG")->name, "Singapore");
    EXPECT_EQ(find("US")->name, "United States");
    EXPECT_LT(find("AT"), find("DE"));
    EXPECT_EQ(Region::catalog("de_DE").front().code, "AF");
}

TEST(Region, SearchKeepsTheCurrentRegion)
{
    LanguageConfig config({"en_US"});
    base::Settings(config.path).setValue("Region/region", "SG");
    Region region(config.path);
    EXPECT_EQ(region.region(), "SG");
    region.loadRegions();
    ASSERT_TRUE(waitFor([&] { return !region.isLoading() && !region.availableRegions().empty(); }));
    region.search("  singa ");
    ASSERT_EQ(region.availableRegions().size(), 1u);
    EXPECT_EQ(region.availableRegions().front().code, "SG");
    // English names and codes match in another display language too.
    region.search("de");
    EXPECT_FALSE(region.availableRegions().empty());
    region.search("no-such-region");
    EXPECT_TRUE(region.availableRegions().empty());
}

TEST(Region, SessionLocaleHasTheChoices)
{
    LanguageConfig config({"ja_JP", "en_US"});
    {
        base::Settings settings(config.path);
        settings.setValue("Region/region", "DE");
        settings.setValue("Region/firstWeekday", 7);
        settings.setValue("Region/twentyFourHour", false);
        settings.setValue("Region/temperature", "fahrenheit");
    }
    const base::Locale locale = Region::sessionLocale(config.path);
    EXPECT_EQ(locale.territory(), "DE");
    EXPECT_EQ(locale.language(), "ja");
    EXPECT_EQ(locale.firstDayOfWeek(), 7);
    EXPECT_FALSE(locale.usesTwentyFourHourTime());
    EXPECT_EQ(locale.temperatureUnit(), base::TemperatureUnit::Fahrenheit);
    EXPECT_EQ(locale.measurementSystem(), base::MeasurementSystem::Metric);

    Region region(config.path);
    EXPECT_EQ(region.preferences().firstDayOfWeek, 7);
    EXPECT_EQ(region.locale().formatsId(), locale.formatsId());
}

TEST(Language, SavingWithoutTheServiceReportsAnError)
{
    // Never against the live session, whose languages this would change.
    const base::dbus::Proxy daemon(base::dbus::Connection::session(), "org.freedesktop.DBus",
                                   "/org/freedesktop/DBus", "org.freedesktop.DBus");
    if (daemon.call("NameHasOwner", std::string("org.cutefish.Services")).value<bool>().value_or(false))
        GTEST_SKIP() << "Needs a private session bus: run the tests in dbus-run-session";
    LanguageConfig config({"en_US", "de_DE"});
    Language language(config.path);
    language.moveLanguage(1, 0);
    ASSERT_TRUE(waitFor([&] { return !language.isSaving(); }));
    EXPECT_EQ(language.errorMessage(), "Could not save preferred languages. Please try again.");
    EXPECT_EQ(codes(language.preferredLanguages()), (std::vector<std::string>{"en_US", "de_DE"}));
}
