// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/language/region.h"

#include <algorithm>
#include <map>
#include <memory>
#include <mutex>

#include <unicode/locid.h>
#include <unicode/unistr.h>

#include "base/paths.h"
#include "base/strings.h"
#include "pages/language/language.h"

using namespace cutefish::base;

namespace settings {

static constexpr const char* kService = "org.cutefish.Services";
static constexpr const char* kPath = "/org/cutefish/Services/Language";
static constexpr const char* kInterface = "org.cutefish.Services.Language";

static icu::UnicodeString folded(const std::string& text)
{
    icu::UnicodeString string = icu::UnicodeString::fromUTF8(text);
    string.foldCase();
    return string;
}

static std::string territoryName(const std::string& code, const icu::Locale& displayLocale)
{
    icu::UnicodeString name;
    icu::Locale("", code.c_str()).getDisplayCountry(displayLocale, name);
    std::string utf8;
    name.toUTF8String(utf8);
    return utf8;
}

std::string Region::defaultConfigPath()
{
    return paths::cutefishConfig("services");
}

Locale Region::sessionLocale(const std::string& configPath)
{
    RegionalPreferences preferences = RegionalPreferences::read(configPath);
    if (preferences.region.empty())
        preferences.region = Locale::systemRegion();
    return Locale::regional(Language::preferredLanguageCodes(configPath).front(), preferences);
}

std::string Region::displayName(const std::string& code)
{
    return territoryName(code, icu::Locale(Language::displayLanguage().c_str()));
}

std::vector<RegionEntry> Region::catalog(const std::string& displayCode)
{
    const icu::Locale displayLocale(displayCode.c_str());
    const icu::Locale english("en");
    std::vector<RegionEntry> entries;
    for (const char* const* code = icu::Locale::getISOCountries(); *code; ++code) {
        RegionEntry entry;
        entry.code = *code;
        entry.name = territoryName(entry.code, displayLocale);
        // ICU names a territory it has no name for by its code.
        if (entry.name.empty() || entry.name == entry.code)
            continue;
        entry.searchText = entry.name + ' ' + territoryName(entry.code, english) + ' ' + entry.code;
        entries.push_back(std::move(entry));
    }
    const Locale collation(displayCode);
    std::sort(entries.begin(), entries.end(), [&collation](const RegionEntry& a, const RegionEntry& b) {
        return collation.compare(a.name, b.name) < 0;
    });
    return entries;
}

// Built once per display language and process, as the language catalog.
static std::mutex s_catalogMutex;
static std::map<std::string, std::shared_ptr<const std::vector<RegionEntry>>> s_catalogs;

Region::Region(std::string configPath)
    : m_configPath(std::move(configPath))
    , m_service(dbus::Connection::session(), kService, kPath, kInterface)
{
    reload();
    m_changedMatch = m_service.onSignal("languageChanged", [this](const dbus::Message&) { reload(); });
}

std::string Region::region() const
{
    return m_preferences.region.empty() ? Locale::systemRegion() : m_preferences.region;
}

Locale Region::locale() const
{
    RegionalPreferences preferences = m_preferences;
    preferences.region = region();
    return Locale::regional(Language::preferredLanguageCodes(m_configPath).front(), preferences);
}

void Region::reload()
{
    RegionalPreferences preferences = RegionalPreferences::read(m_configPath);
    if (preferences == m_preferences)
        return;
    m_preferences = std::move(preferences);
    changed.emit();
    availableRegionsChanged.emit();
}

void Region::send(const char* method, const std::string& value)
{
    m_service.send(method, value);
    changed.emit();
}

void Region::send(const char* method, int value)
{
    m_service.send(method, int32_t(value));
    changed.emit();
}

void Region::setRegion(const std::string& code)
{
    if (code == m_preferences.region)
        return;
    m_preferences.region = code;
    send("setRegion", code);
    availableRegionsChanged.emit();
}

void Region::setFirstDayOfWeek(int day)
{
    if (day < 0 || day > 7 || day == m_preferences.firstDayOfWeek)
        return;
    m_preferences.firstDayOfWeek = day;
    send("setFirstWeekday", day);
}

void Region::setMeasurementSystem(std::optional<MeasurementSystem> system)
{
    if (system == m_preferences.measurementSystem)
        return;
    m_preferences.measurementSystem = system;
    static const char* const names[] = {"metric", "us", "uk"};
    send("setMeasurementSystem", system ? std::string(names[int(*system)]) : std::string());
}

void Region::setTemperatureUnit(std::optional<TemperatureUnit> unit)
{
    if (unit == m_preferences.temperatureUnit)
        return;
    m_preferences.temperatureUnit = unit;
    send("setTemperatureUnit",
         unit ? std::string(*unit == TemperatureUnit::Fahrenheit ? "fahrenheit" : "celsius") : std::string());
}

void Region::setTwentyFourHour(std::optional<bool> enabled)
{
    if (enabled == m_preferences.twentyFourHour)
        return;
    m_preferences.twentyFourHour = enabled;
    send("setTwentyFourHour", enabled ? (*enabled ? 2 : 1) : 0);
}

std::vector<RegionEntry> Region::availableRegions() const
{
    const icu::UnicodeString needle = folded(m_search);
    std::vector<RegionEntry> entries;
    for (const RegionEntry& entry : m_catalog) {
        if (!needle.isEmpty() && folded(entry.searchText).indexOf(needle) < 0)
            continue;
        entries.push_back(entry);
    }
    return entries;
}

void Region::loadRegions()
{
    if (m_loading || m_loaded)
        return;
    const std::string displayCode = Language::displayLanguage();
    {
        std::lock_guard lock(s_catalogMutex);
        if (const auto cached = s_catalogs.find(displayCode); cached != s_catalogs.end()) {
            m_catalog = *cached->second;
            m_loaded = true;
            availableRegionsChanged.emit();
            return;
        }
    }
    m_loading = true;
    loadingChanged.emit();
    m_catalogJob = runInThread(
            [displayCode] {
                auto entries = std::make_shared<const std::vector<RegionEntry>>(catalog(displayCode));
                std::lock_guard lock(s_catalogMutex);
                return s_catalogs.emplace(displayCode, std::move(entries)).first->second;
            },
            [this](std::shared_ptr<const std::vector<RegionEntry>> entries) {
                m_catalog = *entries;
                m_loaded = true;
                m_loading = false;
                loadingChanged.emit();
                availableRegionsChanged.emit();
            });
}

void Region::search(const std::string& text)
{
    const std::string search(strings::trimmed(text));
    if (search == m_search)
        return;
    m_search = search;
    availableRegionsChanged.emit();
}

} // namespace settings
