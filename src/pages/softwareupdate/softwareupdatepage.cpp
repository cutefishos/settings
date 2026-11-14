// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/application_registry.h"
#include "base/desktop_entry.h"
#include "base/i18n.h"
#include "base/signal.h"
#include "cutefish/packagekit/updates.h"
#include "pages/softwareupdate/softwareupdate.h"
#include "ui/button.h"
#include "ui/dialog_window.h"
#include "ui/header_button.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/progress_bar.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;
using cutefish::i18n::format;
using cutefish::i18n::tr;
using cutefish::i18n::trn;

namespace settings {

namespace {

// A label in the theme's secondary text colour.
class SecondaryLabel : public Label
{
public:
    using Label::Label;

protected:
    void layout() override
    {
        setColor(Theme::current().disabledText);
        Label::layout();
    }
};

// The icon of the application a package installs, when an entry is named
// after the package or names it as its icon or program; a package's icon
// otherwise.
static std::string packageIcon(const std::string& package)
{
    const base::ApplicationRegistry& registry = base::ApplicationRegistry::instance();
    if (const auto entry = registry.installedById(package); entry && !entry->icon.empty())
        return entry->icon;
    for (const auto& entry : registry.applications()) {
        if (entry->icon.empty())
            continue;
        const std::vector<std::string> command = base::DesktopEntry::parseExec(entry->exec);
        if (entry->icon == package
            || (!command.empty() && std::filesystem::path(command.front()).filename() == package))
            return entry->icon;
    }
    return "package-x-generic";
}

static constexpr double kStopButton = 24;
static constexpr double kStopGlyph = 16;

// Keeps the stop button's place on the bar's line while the button hides, so
// the bar keeps its length.
class StopSlot : public View
{
public:
    StopSlot()
        : m_button(addChild<HeaderButton>(std::string(), kStopButton))
    {
    }

    HeaderButton* button() const { return m_button; }

    SizeF sizeHint() const override { return {kStopButton, kStopButton}; }

protected:
    void layout() override { m_button->setGeometry(localRect()); }

private:
    HeaderButton* m_button;
};

// Where the updates stand, with the one action that takes them further.
class StatusRow : public ListRow
{
public:
    StatusRow()
    {
        setInteractive(false);
        setMinimumRowHeight(metrics::twoLineRowHeight);
        ColumnLayout* texts = addContent<ColumnLayout>();
        texts->setSpacing(metrics::textSpacing);
        texts->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Center});
        m_title = texts->addChild<Label>();
        m_title->setName("softwareupdate:title");
        m_title->setElide(Label::Elide::Right);
        m_title->setLayoutHints({.fillWidth = true});
        m_progressRow = texts->addChild<RowLayout>();
        m_progressRow->setSpacing(metrics::controlSpacing);
        m_progressRow->setLayoutHints({.fillWidth = true});
        m_progress = m_progressRow->addChild<ProgressBar>();
        m_progress->setName("softwareupdate:progress");
        m_progress->setLayoutHints({.fillWidth = true, .alignment = Alignment::Center});
        StopSlot* slot = m_progressRow->addChild<StopSlot>();
        slot->setLayoutHints({.alignment = Alignment::Center});
        m_stop = slot->button();
        m_stop->setGlyph("stop", kStopGlyph);
        m_stop->setName("softwareupdate:stop");
        m_stop->setToolTip(tr("Stop"));
        m_detail = texts->addChild<SecondaryLabel>();
        m_detail->setName("softwareupdate:detail");
        m_detail->setWrap(true);
        m_detail->setLayoutHints({.fillWidth = true});
        m_busy = addContent<BusyIndicator>();
        m_busy->setLayoutHints({.preferredWidth = 18, .preferredHeight = 18});
        m_action = addContent<Button>();
        m_action->setName("softwareupdate:action");
    }

    Label* title() const { return m_title; }
    RowLayout* progressRow() const { return m_progressRow; }
    ProgressBar* progress() const { return m_progress; }
    HeaderButton* stop() const { return m_stop; }
    Label* detail() const { return m_detail; }
    BusyIndicator* busy() const { return m_busy; }
    Button* action() const { return m_action; }

protected:
    void layout() override
    {
        m_stop->setGlyphColor(Theme::current().disabledText);
        // The stop glyph, not its button, ends at the content edge, and the
        // button reaches past a line as tall as the title's, so the title
        // stays where it is over a second line of text.
        const double overhang = std::max(0.0, (kStopButton - m_title->sizeHint().height) / 2);
        m_progressRow->setPadding({.top = -overhang,
                                   .right = metrics::trailingGlyphMargin("stop", kStopGlyph,
                                                                         (kStopButton - kStopGlyph) / 2),
                                   .bottom = -overhang});
        ListRow::layout();
    }

private:
    Label* m_title;
    RowLayout* m_progressRow;
    ProgressBar* m_progress;
    HeaderButton* m_stop;
    Label* m_detail;
    BusyIndicator* m_busy;
    Button* m_action;
};

// One pending update: its name over its summary, and the new version.
class UpdateRow : public ListRow
{
public:
    explicit UpdateRow(const SoftwareUpdate::Update& update)
    {
        setInteractive(false);
        setMinimumRowHeight(metrics::twoLineRowHeight);
        ThemeIcon* icon = setLeading<ThemeIcon>(packageIcon(update.name), 24);
        icon->setName("softwareupdate:icon:" + update.name);
        ColumnLayout* texts = addContent<ColumnLayout>();
        texts->setSpacing(metrics::textSpacing);
        texts->setLayoutHints({.fillWidth = true, .verticalAlignment = Alignment::Center});
        Label* name = texts->addChild<Label>(update.name);
        name->setElide(Label::Elide::Right);
        name->setLayoutHints({.fillWidth = true});
        Label* summary = texts->addChild<SecondaryLabel>(update.summary);
        summary->setElide(Label::Elide::Right);
        summary->setLayoutHints({.fillWidth = true});
        summary->setVisible(!update.summary.empty());
        addContent<SecondaryLabel>(update.version);
    }
};

class SoftwareUpdatePage : public Page
{
public:
    explicit SoftwareUpdatePage(SettingsWindow& window)
        : Page(window, tr("Software Update"))
    {
        Card* status = content()->addChild<Card>(Card::Style::List);
        status->setName("softwareupdate:status");
        m_status = status->addChild<StatusRow>();
        m_status->action()->onClicked = [this] { act(); };
        m_status->stop()->onClicked = [this] { m_update.cancel(); };

        m_list = content()->addChild<Section>(tr("Updates"));
        m_list->setName("softwareupdate:list");
        m_listCard = m_list->addChild<Card>(Card::Style::List);

        m_subscription = m_update.changed.connect([this] { update(); });
        // Application icons come once the registry has read the desktop entries.
        m_registry = base::ApplicationRegistry::instance().changed.connect([this] {
            m_shownUpdates.clear();
            update();
        });
        update();
    }

private:
    void act()
    {
        switch (m_update.state()) {
        case SoftwareUpdate::State::Available:
            m_update.update();
            break;
        case SoftwareUpdate::State::RestartSuggested:
            confirmRestart();
            break;
        default:
            m_update.check();
            break;
        }
    }

    void confirmRestart()
    {
        m_confirm = std::make_unique<ConfirmDialog>(
                &settingsWindow().window(), tr("Restart Now?"),
                tr("The applications close and the computer starts again, which finishes installing the updates. "
                   "Save your work first."));
        m_confirm->setCancelText(tr("Cancel"));
        // TRANSLATORS: the button that restarts the computer.
        m_confirm->setConfirmText(tr("Restart"));
        m_confirm->onAccepted = [this] { m_update.restart(); };
        m_confirm->show();
    }

    void update()
    {
        const SoftwareUpdate::State state = m_update.state();
        if (state == SoftwareUpdate::State::Checking || state == SoftwareUpdate::State::Updating)
            m_lastBusyState = state;
        const uint64_t count = m_update.updates().size();
        const int progress = m_update.progress();

        std::string title;
        std::string detail;
        std::string action;
        switch (state) {
        case SoftwareUpdate::State::Unknown:
        case SoftwareUpdate::State::Checking:
            title = tr("Checking for updates…");
            break;
        case SoftwareUpdate::State::UpToDate:
            title = tr("The system is up to date");
            action = tr("Check Now");
            break;
        case SoftwareUpdate::State::Available:
            title = trn("{n} update available", "{n} updates available", count);
            detail = tr("You can keep working while they install.");
            action = tr("Update Now");
            break;
        case SoftwareUpdate::State::Updating:
            if (m_update.phase() == SoftwareUpdate::Phase::Installing) {
                // TRANSLATORS: {percent} is how far the update has come, such as 80.
                title = progress >= 0 ? format(tr("Installing updates… {percent}%"),
                                               {{"percent", std::to_string(progress)}})
                                      : std::string(tr("Installing updates…"));
            } else {
                // TRANSLATORS: {percent} is how far the update has come, such as 42.
                title = progress >= 0 ? format(tr("Downloading updates… {percent}%"),
                                               {{"percent", std::to_string(progress)}})
                                      : std::string(tr("Downloading updates…"));
            }
            break;
        case SoftwareUpdate::State::RestartSuggested:
            title = tr("Updates installed");
            detail = tr("Restart the computer to finish installing them.");
            action = tr("Restart");
            break;
        case SoftwareUpdate::State::Error:
            title = m_lastBusyState == SoftwareUpdate::State::Updating ? tr("The updates could not be installed")
                                                                         : tr("Cannot check for updates");
            detail = m_update.error().empty() ? std::string(tr("Check the network connection and try again."))
                                              : m_update.error();
            action = tr("Check Now");
            break;
        }

        m_status->title()->setText(title);
        m_status->detail()->setText(detail);
        m_status->detail()->setVisible(!detail.empty());
        m_status->progressRow()->setVisible(state == SoftwareUpdate::State::Updating);
        m_status->progress()->setIndeterminate(progress < 0);
        m_status->progress()->setValue(std::max(0, progress) / 100.0);
        m_status->stop()->setVisible(m_update.canCancel());
        m_status->busy()->setVisible(state == SoftwareUpdate::State::Unknown
                                     || state == SoftwareUpdate::State::Checking);
        m_status->action()->setText(action);
        m_status->action()->setVisible(!action.empty());

        const bool listed = state == SoftwareUpdate::State::Available || state == SoftwareUpdate::State::Updating;
        m_list->setVisible(listed && count > 0);
        if (m_update.updates() != m_shownUpdates) {
            m_shownUpdates = m_update.updates();
            for (ListRow* row : m_rows)
                m_listCard->takeChild(row);
            m_rows.clear();
            for (const SoftwareUpdate::Update& update : m_shownUpdates)
                m_rows.push_back(m_listCard->addChild<UpdateRow>(update));
        }
    }

    SoftwareUpdate m_update;
    base::Subscription m_subscription;
    base::Subscription m_registry;
    SoftwareUpdate::State m_lastBusyState = SoftwareUpdate::State::Checking;
    std::vector<SoftwareUpdate::Update> m_shownUpdates;

    StatusRow* m_status;
    Section* m_list;
    Card* m_listCard;
    std::vector<ListRow*> m_rows;
    std::unique_ptr<ConfirmDialog> m_confirm;
};

// The page is there while PackageKit is on the system bus or can be
// started there.
class PackageKitWatch : public AvailabilityWatch
{
public:
    explicit PackageKitWatch(AvailabilityCallback callback)
        : m_availability(SoftwareUpdate::packageKitBus(), std::move(callback))
    {
    }

private:
    packagekit::Availability m_availability;
};

} // namespace

static const PageRegistration registration(
        "softwareupdate",
        [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<SoftwareUpdatePage>(window); },
        [](SettingsWindow&, AvailabilityCallback callback) -> std::unique_ptr<AvailabilityWatch> {
            return std::make_unique<PackageKitWatch>(std::move(callback));
        });

} // namespace settings
