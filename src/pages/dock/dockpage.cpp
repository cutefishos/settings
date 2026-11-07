// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <cmath>
#include <vector>

#include "base/i18n.h"
#include "pages/dock/docksettings.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/segmented_control.h"
#include "widgets/card.h"
#include "widgets/imageselectionitem.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "widgets/sections.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {

namespace {

// Rows below the sliders, near their height so the card keeps one rhythm.
constexpr double kTallRowHeight = 64;

// A segmented control over the right half of the row, as a SliderRow puts its slider.
class SegmentedRow : public ListRow
{
public:
    SegmentedRow(std::string text, std::vector<std::string> items)
        : ListRow(std::move(text))
    {
        setInteractive(false);
        setMinimumRowHeight(kTallRowHeight);
        m_control = addContent<SegmentedControl>(std::move(items));
        m_control->setContentHeight(32);
    }

    SegmentedControl* control() const { return m_control; }

protected:
    void layout() override
    {
        const double width = std::round((geometry().width - 2 * metrics::cardHorizontalPadding) / 2);
        if (m_control->layoutHints().preferredWidth != width)
            m_control->setLayoutHints({.preferredWidth = width});
        ListRow::layout();
    }

private:
    SegmentedControl* m_control;
};

class DockPage : public Page
{
public:
    explicit DockPage(SettingsWindow& window)
        : Page(window, tr("Dock & Desktop"))
    {
        Section* position = content()->addChild<Section>(tr("Dock"));
        Card* positions = position->addChild<Card>();
        RowLayout* tiles = positions->addChild<RowLayout>();
        tiles->setSpacing(metrics::tileSpacing);
        const char* const names[] = {tr("Left"), tr("Bottom"), tr("Right")};
        const char* const pictures[] = {"images/dock_left.svg", "images/dock_bottom.svg", "images/dock_right.svg"};
        for (int i = 0; i < 3; ++i) {
            ImageSelectionItem* tile = tiles->addChild<ImageSelectionItem>(dataPath(pictures[i]), names[i],
                                                                           SizeF{80, 80});
            tile->onClicked = [this, i] {
                m_settings.setDirection(i);
                updateDirection();
            };
            m_directions.push_back(tile);
        }
        updateDirection();

        Card* sliders = content()->addChild<Card>(Card::Style::List);
        SliderRow* size = sliders->addChild<SliderRow>(tr("Size"));
        size->setName("dock:size");
        size->setCaptions(tr("Small"), tr("Huge"));
        // The value is the icon size itself; the dock derives its paddings
        // from it (54 is the design baseline).
        size->setRange(32, 128);
        size->setStepSize(1);
        size->setValue(m_settings.iconSize());
        // The dock reports no changes back, so the row learns the new value
        // here; otherwise it returns to the old one once the move settles.
        size->onMoved = [this, size](double value) {
            m_settings.setIconSize(int(std::round(value)));
            size->setValue(m_settings.iconSize());
        };
        SliderRow* glass = sliders->addChild<SliderRow>(tr("Glass effect"));
        glass->setName("dock:glass");
        glass->setCaptions(tr("More clear"), tr("More tinted"));
        glass->setRange(0, 1);
        glass->setValue(m_settings.glassTint());
        glass->onMoved = [this, glass](double value) {
            m_settings.setGlassTint(value);
            glass->setValue(m_settings.glassTint());
        };

        SegmentedRow* mode = sliders->addChild<SegmentedRow>(
                tr("Display mode"), std::vector<std::string>{tr("Always show"), tr("Always hide"), tr("Smart hide")});
        SegmentedControl* visibility = mode->control();
        visibility->setCurrentIndex(m_settings.visibility());
        visibility->onCurrentChanged = [this](int index) { m_settings.setVisibility(index); };

        SwitchRow* area = sliders->addChild<SwitchRow>(tr("Minimized windows area"));
        area->setMinimumRowHeight(kTallRowHeight);
        area->setChecked(m_settings.minimizedArea());
        area->onToggled = [this](bool checked) { m_settings.setMinimizedArea(checked); };

        Section* desktop = content()->addChild<Section>(tr("Desktop"));
        Card* corners = desktop->addChild<Card>(Card::Style::List);
        NavigationItem* hotCorners = corners->addChild<NavigationItem>(tr("Hot Corners"));
        hotCorners->onClicked = [&window] { window.pushPage("hotcorners"); };
    }

private:
    void updateDirection()
    {
        for (size_t i = 0; i < m_directions.size(); ++i)
            m_directions[i]->setChecked(m_settings.direction() == int(i));
    }

    DockSettings m_settings;
    std::vector<ImageSelectionItem*> m_directions;
};

} // namespace

static const PageRegistration registration("dock", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<DockPage>(window);
});

} // namespace settings
