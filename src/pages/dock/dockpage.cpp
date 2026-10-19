// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <cmath>
#include <vector>

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

namespace settings {

namespace {

class DockPage : public Page
{
public:
    explicit DockPage(SettingsWindow& window)
        : Page(window, "Dock")
    {
        Section* position = content()->addChild<Section>("Position on screen");
        Card* positions = position->addChild<Card>();
        RowLayout* tiles = positions->addChild<RowLayout>();
        tiles->setSpacing(metrics::tileSpacing);
        const char* const names[] = {"Left", "Bottom", "Right"};
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
        SliderRow* size = sliders->addChild<SliderRow>("Size");
        size->setName("dock:size");
        size->setCaptions("Small", "Huge");
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
        SliderRow* glass = sliders->addChild<SliderRow>("Glass effect");
        glass->setName("dock:glass");
        glass->setCaptions("More clear", "More tinted");
        glass->setRange(0, 1);
        glass->setValue(m_settings.glassTint());
        glass->onMoved = [this, glass](double value) {
            m_settings.setGlassTint(value);
            glass->setValue(m_settings.glassTint());
        };

        Section* mode = content()->addChild<Section>("Display mode");
        auto* visibility = mode->addChild<SegmentedControl>(
                std::vector<std::string>{"Always show", "Always hide", "Smart hide"});
        visibility->setLayoutHints({.fillWidth = true});
        visibility->setCurrentIndex(m_settings.visibility());
        visibility->onCurrentChanged = [this](int index) { m_settings.setVisibility(index); };

        Card* corners = content()->addChild<Card>(Card::Style::List);
        NavigationItem* hotCorners = corners->addChild<NavigationItem>("Hot Corners");
        hotCorners->onClicked = [&window] { window.pushPage("hotcorners"); };

        Card* minimized = content()->addChild<Card>(Card::Style::List);
        SwitchRow* area = minimized->addChild<SwitchRow>("Minimized windows area");
        area->setChecked(m_settings.minimizedArea());
        area->onToggled = [this](bool checked) { m_settings.setMinimizedArea(checked); };
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
