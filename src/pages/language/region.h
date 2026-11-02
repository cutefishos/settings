// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/locale.h"
#include "base/signal.h"
#include "base/thread_pool.h"

namespace settings {

struct RegionEntry
{
    // ISO 3166, such as "SG".
    std::string code;
    // In the language of the user interface.
    std::string name;
    std::string searchText;
};

// The session's region and the format choices that go with it, as
// cutefish-services keeps them, and the catalog of regions.
class Region
{
public:
    // Reads cutefish-services' settings at the path.
    explicit Region(std::string configPath = defaultConfigPath());

    // As stored: an empty region or an unset choice follows the system.
    const cutefish::base::RegionalPreferences& preferences() const { return m_preferences; }
    // The chosen region, else the system's.
    std::string region() const;
    // How the session writes dates and numbers now.
    cutefish::base::Locale locale() const;

    void setRegion(const std::string& code);
    // 1 for Monday to 7 for Sunday; 0 for the region's.
    void setFirstDayOfWeek(int day);
    void setMeasurementSystem(std::optional<cutefish::base::MeasurementSystem> system);
    void setTemperatureUnit(std::optional<cutefish::base::TemperatureUnit> unit);
    void setTwentyFourHour(std::optional<bool> enabled);

    // The catalog entries matching the search.
    std::vector<RegionEntry> availableRegions() const;
    bool isLoading() const { return m_loading; }
    // Reads the catalog on a thread, once per display language and process.
    void loadRegions();
    // Keeps catalog entries whose code or names contain the text, ignoring case.
    void search(const std::string& text);

    cutefish::base::Signal<> changed;
    // The catalog arrived or the search changed.
    cutefish::base::Signal<> availableRegionsChanged;
    cutefish::base::Signal<> loadingChanged;

    static std::string defaultConfigPath();
    // The formats of the first preferred language in the stored region,
    // else the system's, with the stored choices.
    static cutefish::base::Locale sessionLocale(const std::string& configPath);
    // A region's name in the language of the user interface.
    static std::string displayName(const std::string& code);
    // Every region CLDR names, sorted by name.
    static std::vector<RegionEntry> catalog(const std::string& displayCode);

private:
    void reload();
    void send(const char* method, const std::string& value);
    void send(const char* method, int value);

    std::string m_configPath;
    cutefish::base::dbus::Proxy m_service;
    cutefish::base::dbus::Match m_changedMatch;
    cutefish::base::Job m_catalogJob;
    cutefish::base::RegionalPreferences m_preferences;
    std::vector<RegionEntry> m_catalog;
    std::string m_search;
    bool m_loaded = false;
    bool m_loading = false;
};

} // namespace settings
