// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <chrono>
#include <cmath>
#include <memory>
#include <string>
#include <vector>

#include "base/event_loop.h"
#include "base/timer.h"
#include "pages/battery/battery.h"
#include "pages/battery/batteryhistory.h"
#include "pages/battery/historygraph.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/segmented_control.h"
#include "ui/theme.h"
#include "cutefish/upower/battery_status.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "window/options.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;

namespace settings {

namespace {

constexpr uint32_t kHistoryDurations[] = {86400, 7 * 86400};

// The section title with the range picker at its end: BatterySection.qml's
// RowLayout.
class HistoryTitle : public RowLayout
{
public:
    HistoryTitle()
    {
        setSpacing(metrics::m);
        setLayoutHints({.fillWidth = true});
        m_title = addChild<SectionLabel>("History");
        m_title->setName("battery:historytitle");
        m_picker = addChild<SegmentedControl>(std::vector<std::string>{"24 hours", "7 days"});
        // Compact, to sit in the title row.
        m_picker->setContentHeight(28);
    }

    SegmentedControl* picker() const { return m_picker; }

protected:
    void layout() override
    {
        // Two equal segments, each wide enough for the longer label. A whole
        // width, as the right edge must meet the card's.
        const double widest = std::max(m_picker->implicitItemWidth(0), m_picker->implicitItemWidth(1));
        const double width = std::ceil(widest * 2 + metrics::l * 4 + m_picker->padding() * 2);
        if (m_picker->layoutHints().preferredWidth != width)
            m_picker->setLayoutHints({.preferredWidth = width});
        RowLayout::layout();
    }

private:
    SectionLabel* m_title;
    SegmentedControl* m_picker;
};

class BatteryPage : public Page
{
public:
    explicit BatteryPage(SettingsWindow& window)
        : Page(window, "Battery")
    {
        // History.
        m_historySection = content()->addChild<ColumnLayout>();
        m_historySection->setName("battery:history");
        m_historySection->setSpacing(metrics::sectionLabelSpacing);
        m_historySection->setLayoutHints({.fillWidth = true});
        HistoryTitle* title = m_historySection->addChild<HistoryTitle>();
        m_rangePicker = title->picker();
        m_rangePicker->setName("battery:range");
        m_rangePicker->onCurrentChanged = [this](int index) {
            BatteryHistory::Query query = m_history.query();
            query.duration = kHistoryDurations[index];
            m_history.load(query);
        };
        Card* graphCard = m_historySection->addChild<Card>();
        graphCard->setSpacing(0);
        graphCard->setName("battery:graphcard");
        m_graph = graphCard->addChild<HistoryGraph>();
        m_graph->setName("battery:graph");
        // `height: 240` on an item without an implicit size.
        m_graph->setLayoutHints({.fillWidth = true, .fallbackHeight = 240});
        m_graph->setValueAxis(100, 25, "%", 0);
        m_graph->setDuration(m_plottedDuration);

        // Health.
        m_health = content()->addChild<Section>("Health");
        m_health->setName("battery:health");
        Card* health = m_health->addChild<Card>(Card::Style::List);
        m_condition = health->addChild<InfoRow>("Condition");
        m_capacity = health->addChild<InfoRow>("Maximum capacity");
        m_lastCharged = health->addChild<InfoRow>();
        m_lastCharged->setName("battery:lastcharged");

        // Charging.
        m_charging = content()->addChild<Section>("Charging");
        m_charging->setName("battery:charging");
        Card* charging = m_charging->addChild<Card>(Card::Style::List);
        m_chargeLimit = charging->addChild<SwitchRow>();
        m_chargeLimit->setName("battery:chargelimit");
        m_chargeLimit->onToggled = [this](bool checked) { m_battery.setChargeLimitEnabled(checked); };
        m_chargeLimitNote = m_charging->addChild<Footnote>();
        m_chargeLimitNote->setName("battery:chargelimitnote");

        Card* percentage = content()->addChild<Card>(Card::Style::List);
        SwitchRow* showPercent = percentage->addChild<SwitchRow>("Show percentage in status bar");
        showPercent->setChecked(m_battery.showPercent());
        showPercent->onToggled = [this](bool checked) { m_battery.setPercentEnabled(checked); };

        m_subscriptions.push_back(m_battery.capacityChanged.connect([this] { updateHealth(); }));
        m_subscriptions.push_back(m_battery.lastChargedChanged.connect([this] { updateHealth(); }));
        m_subscriptions.push_back(m_battery.chargeLimitChanged.connect([this] { updateChargeLimit(); }));
        m_subscriptions.push_back(m_battery.udiChanged.connect([this] {
            BatteryHistory::Query query = m_history.query();
            query.device = m_battery.udi();
            m_history.load(query);
        }));
        m_subscriptions.push_back(m_battery.availableChanged.connect([this] { updateRefreshTimer(); }));
        m_subscriptions.push_back(window.activeChanged.connect([this] { updateRefreshTimer(); }));
        m_subscriptions.push_back(m_history.samplesChanged.connect([this] { historyChanged(); }));

        // Picks up new samples and moves the plot's right edge along with the
        // clock, only while the page is in front.
        m_refresh.onTimeout = [this] { m_history.refresh(); };

        m_history.load({m_battery.udi(), kHistoryDurations[0], BatteryHistory::Type::Charge});

        m_historySection->setVisible(false);
        updateHealth();
        updateChargeLimit();
        updateRefreshTimer();
    }

private:
    void historyChanged()
    {
        // The range the plotted samples cover follows the picker only once
        // they arrive, so the old samples are never drawn across the new range.
        m_plottedDuration = m_history.query().duration;
        std::vector<HistoryGraph::Point> points;
        points.reserve(m_history.samples().size());
        for (const auto& sample : m_history.samples())
            points.push_back({double(sample.time), sample.value, sample.state});
        m_graph->setPoints(std::move(points));
        m_graph->setEnd(m_history.samples().empty() ? 0 : m_history.samples().back().time);
        m_graph->setDuration(m_plottedDuration);
        m_historySection->setVisible(m_history.samples().size() > 2);
    }

    void updateRefreshTimer()
    {
        const bool running = m_battery.isAvailable() && settingsWindow().isActive();
        if (running == m_refresh.isActive())
            return;
        if (!running) {
            m_refresh.stop();
            return;
        }
        m_refresh.start(std::chrono::minutes(1));
        // The history loads by itself at first, so only a return needs a refresh.
        if (!m_history.samples().empty())
            m_history.refresh();
    }

    void updateHealth()
    {
        const int capacity = m_battery.capacity();
        m_health->setVisible(capacity != 0);
        m_condition->setValue(capacity >= 90 ? "Excellent" : capacity >= 80 ? "Normal" : "Poor");
        m_capacity->setValue(std::to_string(capacity) + "%");
        const int lastCharged = m_battery.lastChargedPercent();
        m_lastCharged->setText("Last charged to " + std::to_string(lastCharged) + "%");
        m_lastCharged->setValue(m_battery.lastChargedTime());
        m_lastCharged->setVisible(lastCharged != 0);
    }

    void updateChargeLimit()
    {
        m_charging->setVisible(m_battery.chargeLimitSupported());
        m_chargeLimit->setText("Limit charging to " + std::to_string(m_battery.chargeLimitEnd()) + "%");
        m_chargeLimit->setChecked(m_battery.chargeLimitEnabled());
        updateChargeLimitNote();
    }

    void updateChargeLimitNote()
    {
        const bool held = m_battery.chargeLimitHeld();
        m_chargeLimitNote->setText(
                held ? "While plugged in, charging stops at " + std::to_string(m_battery.chargeLimitEnd())
                               + "% and resumes below " + std::to_string(m_battery.chargeLimitStart())
                               + "%, so the battery wears more slowly."
                     : "This computer's battery controller did not keep the limit, so the battery still "
                       "charges fully.");
        m_chargeLimitNote->setColor(held ? std::nullopt : std::optional<Color>(accent::orange));
    }

    Battery m_battery;
    BatteryHistory m_history;
    base::Timer m_refresh;
    double m_plottedDuration = kHistoryDurations[0];
    std::vector<base::Subscription> m_subscriptions;

    ColumnLayout* m_historySection;
    SegmentedControl* m_rangePicker;
    HistoryGraph* m_graph;
    Section* m_health;
    InfoRow* m_condition;
    InfoRow* m_capacity;
    InfoRow* m_lastCharged;
    Section* m_charging;
    SwitchRow* m_chargeLimit;
    Footnote* m_chargeLimitNote;
};

// The page is there while cutefish-services reports a battery; until the
// service has answered, there is none either way.
class BatteryWatch : public AvailabilityWatch
{
public:
    explicit BatteryWatch(AvailabilityCallback callback)
        : m_callback(std::move(callback))
    {
        if (options().mockBattery) {
            answerLater(true);
            return;
        }
        m_status = upower::BatteryStatus::shared();
        if (m_status->isReady()) {
            answerLater(m_status->isAvailable());
            return;
        }
        m_ready = m_status->readyChanged.connect([this] { m_callback(m_status->isAvailable()); });
    }

private:
    // Not from inside the watch's creation, which the window is still busy with.
    void answerLater(bool available)
    {
        m_answer.setSingleShot(true);
        m_answer.onTimeout = [this, available] { m_callback(available); };
        m_answer.start(std::chrono::milliseconds(0));
    }

    AvailabilityCallback m_callback;
    std::shared_ptr<upower::BatteryStatus> m_status;
    base::Subscription m_ready;
    base::Timer m_answer;
};

} // namespace

static const PageRegistration registration(
        "battery", [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<BatteryPage>(window); },
        [](SettingsWindow&, AvailabilityCallback callback) -> std::unique_ptr<AvailabilityWatch> {
            return std::make_unique<BatteryWatch>(std::move(callback));
        });

} // namespace settings
