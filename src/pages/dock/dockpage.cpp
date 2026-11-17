// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <algorithm>
#include <cmath>
#include <vector>

#include "base/i18n.h"
#include "cutefish/appearance/appearance.h"
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
using cutefish::appearance::Appearance;
using cutefish::i18n::tr;

namespace settings {

namespace {

// Rows below the sliders, near their height so the card keeps one rhythm.
constexpr double kTallRowHeight = 64;
// The least room beside the label inside each button of a segmented row.
constexpr double kSegmentPadding = 8;

// The magnification slider: off at its left end, then the magnified sizes;
// below kMagnificationOn a drag is off, from there up it is at least the
// smallest size.
constexpr int kMagnificationOff = 56;
constexpr int kMagnificationOn = 60;
constexpr int kSmallestMagnified = 64;
constexpr int kLargestMagnified = 128;

double magnificationValue(int size)
{
    return size <= 0 ? kMagnificationOff : size;
}

// A segmented control over the right half of the row, as a SliderRow puts its
// slider, or under the label across the row once half the row leaves its
// labels too little room.
class SegmentedRow : public ListRow
{
public:
    SegmentedRow(std::string text, std::vector<std::string> items)
        : ListRow(std::move(text))
    {
        setInteractive(false);
        setMinimumRowHeight(kTallRowHeight);
        m_control = addChild<SegmentedControl>(std::move(items));
        m_control->setContentHeight(32);
    }

    SegmentedControl* control() const { return m_control; }

    double heightForWidth(double width) const override
    {
        const double content = contentWidth(width);
        if (isStacked(content))
            return labelRowHeight(content, metrics::rowHeight) + m_control->sizeHint().height
                   + metrics::cardVerticalPadding;
        return labelRowHeight(content - inlineWidth(content) - metrics::rowSpacing, kTallRowHeight);
    }

protected:
    void layout() override
    {
        const double content = contentWidth(geometry().width);
        const bool stacked = isStacked(content);
        const double width = stacked ? content : inlineWidth(content);
        LayoutHints hints = label()->layoutHints();
        hints.margins.right = stacked ? 0 : width + metrics::rowSpacing;
        if (!(hints.margins == label()->layoutHints().margins))
            label()->setLayoutHints(hints);
        if (stacked != m_stacked) {
            m_stacked = stacked;
            setMinimumRowHeight(stacked ? metrics::rowHeight : kTallRowHeight);
        }
        ListRow::layout();

        const double height = m_control->sizeHint().height;
        const double row = rowHeight();
        const double x = metrics::cardHorizontalPadding + content - width;
        const double y = stacked ? row : std::round((row - height) / 2);
        m_control->setGeometry({x, y, width, height});
    }

private:
    static double contentWidth(double width) { return std::max(0.0, width - 2 * metrics::cardHorizontalPadding); }
    static double inlineWidth(double content) { return std::round(content / 2); }

    // The buttons share the width equally, so each needs the widest label.
    bool isStacked(double content) const
    {
        double widest = 0;
        for (size_t i = 0; i < m_control->count(); ++i)
            widest = std::max(widest, m_control->implicitItemWidth(i));
        return m_control->count() * (widest + 2 * kSegmentPadding) + 2 * m_control->padding() > inlineWidth(content);
    }

    double labelRowHeight(double labelWidth, double minimum) const
    {
        return std::max(minimum, label()->heightForWidth(std::max(0.0, labelWidth)) + metrics::cardVerticalPadding);
    }

    SegmentedControl* m_control;
    bool m_stacked = false;
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
        size->setCaptions(tr("Small"), tr("Large"));
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
        SliderRow* magnification = sliders->addChild<SliderRow>(tr("Magnification"));
        magnification->setName("dock:magnification");
        magnification->setCaptions(tr("Off"), tr("Large"));
        // The size the icon under the pointer grows to.
        magnification->setRange(kMagnificationOff, kLargestMagnified);
        magnification->setStepSize(1);
        magnification->setValue(magnificationValue(m_settings.magnifiedIconSize()));
        magnification->onMoved = [this, magnification](double value) {
            const int position = int(std::round(value));
            m_settings.setMagnifiedIconSize(position < kMagnificationOn ? 0 : std::max(kSmallestMagnified, position));
            magnification->setValue(magnificationValue(m_settings.magnifiedIconSize()));
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
        // Reduce Transparency gives the dock a grey of its own.
        m_glass = glass;
        m_subscription = m_appearance.changed.connect([this] { updateGlass(); });
        updateGlass();

        SegmentedRow* mode = sliders->addChild<SegmentedRow>(
                tr("Display mode"), std::vector<std::string>{tr("Always show"), tr("Always hide"), tr("Smart hide")});
        SegmentedControl* visibility = mode->control();
        visibility->setName("dock:displayMode");
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
    void updateGlass() { m_glass->setEnabled(!m_appearance.reduceTransparency()); }

    void updateDirection()
    {
        for (size_t i = 0; i < m_directions.size(); ++i)
            m_directions[i]->setChecked(m_settings.direction() == int(i));
    }

    DockSettings m_settings;
    Appearance m_appearance;
    cutefish::base::Subscription m_subscription;
    std::vector<ImageSelectionItem*> m_directions;
    SliderRow* m_glass;
};

} // namespace

static const PageRegistration registration("dock", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<DockPage>(window);
});

} // namespace settings
