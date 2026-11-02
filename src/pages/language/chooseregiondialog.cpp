// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/language/chooseregiondialog.h"

#include <chrono>
#include <functional>

#include "base/i18n.h"
#include "pages/language/region.h"
#include "ui/dialog_window.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/scroll_view.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/emptylistoverlay.h"
#include "widgets/listrow.h"
#include "widgets/searchfield.h"

using namespace cutefish;
using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {

namespace {

// A region to pick, with a check when it is the chosen one.
class RegionRow : public ListRow
{
public:
    RegionRow(const RegionEntry& entry, bool checked)
        : ListRow(entry.name)
        , code(entry.code)
    {
        m_check = addContent<TrailingIcon>("check", 18);
        m_check->setVisible(checked);
        setName("region:" + entry.code);
    }

    const std::string code;

    void setChecked(bool checked)
    {
        m_check->setVisible(checked);
        requestLayout();
    }

protected:
    void layout() override
    {
        m_check->icon()->setColor(Theme::current().highlight);
        ListRow::layout();
    }

private:
    TrailingIcon* m_check;
};

// The rows; once laid out, scrolls a row it was given to the middle.
class RegionColumn : public ColumnLayout
{
public:
    explicit RegionColumn(ScrollView* scroll)
        : m_scroll(scroll)
    {
    }

    // Null before the rows are replaced.
    void reveal(View* row)
    {
        m_reveal = row;
        if (row)
            requestLayout();
    }

    std::function<void()> onRevealed;

protected:
    void layout() override
    {
        ColumnLayout::layout();
        if (!m_reveal)
            return;
        const RectF row = m_reveal->geometry();
        const double viewport = m_scroll->geometry().height;
        m_scroll->ensureVisible({row.x, row.y + row.height / 2 - viewport / 2, row.width, viewport});
        m_reveal = nullptr;
        if (onRevealed)
            onRevealed();
    }

private:
    ScrollView* m_scroll;
    View* m_reveal = nullptr;
};

} // namespace

ChooseRegionDialog::ChooseRegionDialog(SettingsWindow& parent, Region& region)
    : FormDialog(parent, tr("Choose Region"), 460)
    , m_region(region)
    , m_selectedCode(region.region())
{
    setCloseVisible(false);
    setFixedHeight(560);
    // The page colour, so the card stands out as it does on a page.
    setBackgroundColor(ColorId::Background);

    m_search = content()->addChild<SearchField>();
    m_search->setPlaceholder(tr("Search regions"));
    m_search->setLayoutHints({.fillWidth = true});
    m_search->onTextChanged = [this] { m_searchTimer.start(); };
    m_searchTimer.setSingleShot(true);
    m_searchTimer.setInterval(std::chrono::milliseconds(100));
    m_searchTimer.onTimeout = [this] {
        m_region.search(m_search->text());
        m_scroll->setScrollPosition({0, 0});
    };

    Card* card = content()->addChild<Card>(Card::Style::List);
    card->setLayoutHints({.fillWidth = true, .fillHeight = true});
    StackView* stack = card->addChild<StackView>();
    stack->setLayoutHints({.fillWidth = true, .fillHeight = true});
    stack->setClipsChildren(true);
    m_scroll = stack->addChild<ScrollView>();
    m_scroll->setElastic(false);
    RegionColumn* column = m_scroll->setContent<RegionColumn>(m_scroll);
    column->onRevealed = [this] { m_revealSelection = false; };
    m_rows = column;
    m_rows->setSpacing(0);
    m_empty = stack->addChild<EmptyListOverlay>()->label();

    RowLayout* buttons = addButtonRow();
    DialogButton* cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    cancel->onClicked = [this] { requestClose(); };
    m_choose = buttons->addChild<DialogButton>(tr("Choose"), true);
    m_choose->onClicked = [this] {
        m_region.setRegion(m_selectedCode);
        close();
    };

    m_subscriptions.push_back(m_region.availableRegionsChanged.connect([this] { updateRows(); }));
    m_subscriptions.push_back(m_region.loadingChanged.connect([this] { updateRows(); }));
    onShown = [this] { m_search->setFocus(); };

    m_region.search({});
    m_region.loadRegions();
    updateRows();
}

void ChooseRegionDialog::select(const std::string& code)
{
    m_selectedCode = code;
    for (const auto& child : m_rows->children()) {
        auto* row = static_cast<RegionRow*>(child.get());
        row->setChecked(row->code == code);
    }
    updateStates();
}

void ChooseRegionDialog::updateRows()
{
    const PointF position = m_scroll->scrollPosition();
    static_cast<RegionColumn*>(m_rows)->reveal(nullptr);
    while (!m_rows->children().empty())
        m_rows->takeChild(m_rows->children().front().get());
    const std::vector<RegionEntry> regions = m_region.availableRegions();
    RegionRow* selected = nullptr;
    for (const RegionEntry& entry : regions) {
        RegionRow* row = m_rows->addChild<RegionRow>(entry, entry.code == m_selectedCode);
        const std::string code = entry.code;
        row->onClicked = [this, code] { select(code); };
        if (entry.code == m_selectedCode)
            selected = row;
    }
    m_scroll->setScrollPosition(position);
    // The current region in view when the list first fills.
    if (m_revealSelection && selected)
        static_cast<RegionColumn*>(m_rows)->reveal(selected);
    m_empty->setText(m_region.isLoading() ? tr("Loading regions…") : tr("No regions found"));
    m_empty->parent()->setVisible(regions.empty());
    updateStates();
}

void ChooseRegionDialog::updateStates()
{
    m_choose->setEnabled(!m_selectedCode.empty() && m_selectedCode != m_region.region());
}

} // namespace settings
