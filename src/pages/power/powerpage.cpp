// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "pages/power/powersettings.h"
#include "powerprofiles/powerprofiles.h"
#include "ui/layout.h"
#include "ui/quick_layout.h"
#include "upower/batterystatus.h"
#include "widgets/card.h"
#include "widgets/imageselectionitem.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/options.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;

namespace settings {

namespace {

const std::vector<int> kTimeouts{60, 120, 300, 600, 900, 1200, 1800, -1};
const std::vector<std::string> kTimeoutNames{"1 minute",   "2 minutes",  "5 minutes",  "10 minutes",
                                             "15 minutes", "20 minutes", "30 minutes", "Never"};

int timeoutToIndex(int timeout)
{
    auto it = std::find(kTimeouts.begin(), kTimeouts.end(), timeout);
    if (it == kTimeouts.end())
        it = std::find(kTimeouts.begin(), kTimeouts.end(), 300);
    return int(it - kTimeouts.begin());
}

struct ProfileTile
{
    const char* profile;
    const char* picture;
    const char* name;
};

constexpr ProfileTile kProfileTiles[] = {
    {"power-saver", "images/powersave.svg", "Power save"},
    {"balanced", "images/balanced.svg", "Balanced"},
    {"performance", "images/performance.svg", "Performance"},
};

class PowerPage : public Page
{
public:
    explicit PowerPage(SettingsWindow& window)
        : Page(window, "Power")
    {
        // Mode.
        m_mode = content()->addChild<Section>("Mode");
        m_mode->setName("power:mode");
        Card* modes = m_mode->addChild<Card>();
        RowLayout* tiles = modes->addChild<RowLayout>();
        tiles->setSpacing(metrics::tileSpacing);
        for (const ProfileTile& tile : kProfileTiles) {
            ImageSelectionItem* item = tiles->addChild<ImageSelectionItem>(dataPath(tile.picture), tile.name,
                                                                           SizeF{80, 80});
            const std::string profile = tile.profile;
            item->onClicked = [this, profile] { m_profiles.setActiveProfile(profile); };
            m_tiles.push_back(item);
        }
        m_description = m_mode->addChild<Footnote>();
        m_description->setName("power:description");
        m_degraded = m_mode->addChild<Footnote>();
        m_degraded->setName("power:degraded");

        // Display and sleep.
        Card* idle = content()->addChild<Card>(Card::Style::List);
        m_batteryRow = idle->addChild<ListRow>("Turn off display when inactive on battery");
        m_batteryRow->setName("power:batteryscreenoff");
        m_batteryRow->setInteractive(false);
        RowComboBox* batteryScreenOff = m_batteryRow->addContent<RowComboBox>(kTimeoutNames);
        batteryScreenOff->setCurrentIndex(timeoutToIndex(m_settings.batteryScreenOff()));
        batteryScreenOff->onActivated = [this](int index) { m_settings.setBatteryScreenOff(kTimeouts[index]); };

        m_acRow = idle->addChild<ListRow>();
        m_acRow->setName("power:acscreenoff");
        m_acRow->setInteractive(false);
        RowComboBox* acScreenOff = m_acRow->addContent<RowComboBox>(kTimeoutNames);
        acScreenOff->setCurrentIndex(timeoutToIndex(m_settings.acScreenOff()));
        acScreenOff->onActivated = [this](int index) { m_settings.setACScreenOff(kTimeouts[index]); };

        m_preventSleep = idle->addChild<SwitchRow>();
        m_preventSleep->setName("power:preventsleep");
        m_preventSleep->setChecked(m_settings.acPreventSleep());
        m_preventSleep->onToggled = [this](bool checked) { m_settings.setACPreventSleep(checked); };

        if (!options().mockBattery) {
            m_batteryStatus = upower::BatteryStatus::shared();
            m_subscriptions.push_back(m_batteryStatus->availableChanged.connect([this] { updateBattery(); }));
        }

        m_subscriptions.push_back(m_profiles.profilesChanged.connect([this] { updateProfiles(); }));
        m_subscriptions.push_back(m_profiles.activeProfileChanged.connect([this] { updateProfiles(); }));
        m_subscriptions.push_back(m_profiles.performanceDegradedChanged.connect([this] { updateProfiles(); }));
        updateProfiles();
        updateBattery();
    }

private:
    bool hasBattery() const { return options().mockBattery || (m_batteryStatus && m_batteryStatus->isAvailable()); }

    void updateProfiles()
    {
        const std::vector<std::string>& profiles = m_profiles.profiles();
        const std::string& active = m_profiles.activeProfile();
        m_mode->setVisible(!profiles.empty());
        for (size_t i = 0; i < m_tiles.size(); ++i) {
            const std::string profile = kProfileTiles[i].profile;
            m_tiles[i]->setVisible(std::find(profiles.begin(), profiles.end(), profile) != profiles.end());
            m_tiles[i]->setChecked(active == profile);
        }

        std::string description;
        if (active == "power-saver")
            description = "Reduces performance and power consumption.";
        else if (active == "balanced")
            description = "Standard performance and power consumption.";
        else if (active == "performance")
            description = "Higher performance, with more power consumption and heat.";
        m_description->setText(description);
        m_description->setVisible(!description.empty());

        const std::string& degraded = m_profiles.performanceDegraded();
        if (degraded.find("lap-detected") != std::string::npos)
            m_degraded->setText("Performance is limited because the computer is on your lap.");
        else if (degraded.find("high-operating-temperature") != std::string::npos)
            m_degraded->setText("Performance is limited because the computer is too hot.");
        else
            m_degraded->setText("Performance is currently limited.");
        m_degraded->setVisible(active == "performance" && !degraded.empty());
    }

    void updateBattery()
    {
        const bool battery = hasBattery();
        m_batteryRow->setVisible(battery);
        m_acRow->setText(battery ? "Turn off display when inactive on power adapter"
                                 : "Turn off display when inactive");
        m_preventSleep->setText(battery ? "Prevent automatic sleeping on power adapter when the display is off"
                                        : "Prevent automatic sleeping when the display is off");
    }

    powerprofiles::PowerProfiles m_profiles;
    PowerSettings m_settings;
    std::shared_ptr<upower::BatteryStatus> m_batteryStatus;
    std::vector<base::Subscription> m_subscriptions;

    Section* m_mode;
    std::vector<ImageSelectionItem*> m_tiles;
    Footnote* m_description;
    Footnote* m_degraded;
    ListRow* m_batteryRow;
    ListRow* m_acRow;
    SwitchRow* m_preventSleep;
};

} // namespace

static const PageRegistration registration("power", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<PowerPage>(window);
});

} // namespace settings
