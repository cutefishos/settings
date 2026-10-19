// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "pages/datetime/timezonemap.h"

namespace cutefish::ui {
class Window;
}

namespace settings {

class SettingsWindow;

// The world map to pick a time zone on: every zone a dot, the selection a
// pin with a card naming it, search by city or country, and the selection's
// clock and its difference from the zone in use along the bottom.
class TimeZoneDialog
{
public:
    TimeZoneDialog(SettingsWindow& parent, TimeZoneMap& map);
    ~TimeZoneDialog();

    TimeZoneDialog(const TimeZoneDialog&) = delete;
    TimeZoneDialog& operator=(const TimeZoneDialog&) = delete;

    cutefish::ui::Window* window() const { return m_window.get(); }

    void show();
    // The window goes once the event being handled is over.
    void close();
    std::function<void()> onClosed;

    // What the views show.
    TimeZoneMap& map() const { return m_map; }
    const std::string& selectedZone() const { return m_selectedZone; }
    const std::vector<std::string>& nearbyZones() const { return m_nearbyZones; }
    const std::string& hoveredZone() const { return m_hoveredZone; }
    const TimeZoneMap::Info& selectedInfo() const { return m_selectedInfo; }
    // Only a change is worth applying.
    bool isChanged() const;
    bool animatesPin() const { return m_animatePin; }

    // The user's own picks: these slide the pin over. A result is the one
    // current in the search results.
    void pickResult();
    void pickNearby(const std::string& id);
    void mapClicked(int x, int y, int width, int height);
    void setHoveredZone(std::string id);
    void apply();

    // After the selection, its candidates, the hovered zone or the clock changed.
    cutefish::base::Signal<> changed;
    // A new selection, or the dialog shown: the pin ripples.
    cutefish::base::Signal<> selectionStarted;

private:
    class Root;

    void updateInfo();

    TimeZoneMap& m_map;
    std::unique_ptr<cutefish::ui::Window> m_window;
    Root* m_root = nullptr;
    std::vector<cutefish::base::Subscription> m_subscriptions;
    // Keeps the times shown up with the clock while open.
    cutefish::base::Timer m_tick;
    std::string m_selectedZone;
    std::vector<std::string> m_nearbyZones;
    std::string m_hoveredZone;
    TimeZoneMap::Info m_selectedInfo;
    // Opening the dialog puts the pin in place without sliding it in.
    bool m_animatePin = false;
};

} // namespace settings
