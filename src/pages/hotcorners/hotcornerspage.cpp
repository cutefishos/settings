// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <array>

#include "pages/hotcorners/hotcorners.h"
#include "ui/label.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish::ui;

namespace settings {

namespace {

// The compositor's action names, "" for none, and what the menu calls them.
const std::array<const char*, 5> kActions = {"", "toggle-overview", "toggle-show-desktop", "toggle-app-list",
                                             "lock-screen"};
const std::vector<std::string> kActionNames = {"None", "Show All Windows", "Show Desktop", "Launcher",
                                               "Lock Screen"};

class HotCornersPage : public Page
{
public:
    explicit HotCornersPage(SettingsWindow& window)
        : Page(window, "Hot Corners")
    {
        m_unavailable = content()->addChild<Card>();
        m_unavailableLabel = m_unavailable->addChild<Label>(
                "Hot corners are managed by the Cutefish compositor, which is not running.");
        m_unavailableLabel->setWrap(true);
        m_unavailableLabel->setLayoutHints({.fillWidth = true});

        m_list = content()->addChild<Card>(Card::Style::List);
        const std::array<std::pair<const char*, const char*>, 4> corners = {
                std::pair{"Top left", "top-left"}, {"Top right", "top-right"}, {"Bottom left", "bottom-left"},
                {"Bottom right", "bottom-right"}};
        for (const auto& [text, corner] : corners) {
            ListRow* row = m_list->addChild<ListRow>(text);
            row->setInteractive(false);
            RowComboBox* box = row->addContent<RowComboBox>(kActionNames);
            const std::string name = corner;
            // Keys held while choosing are the ones the corner will ask for.
            box->onActivated = [this, name](int index) {
                m_corners.setCorner(name, kActions[size_t(index)], index > 0 ? m_corners.heldModifiers() : 0);
            };
            m_boxes.push_back({name, box});
        }
        m_note = content()->addChild<Footnote>(
                "Move the pointer into a corner of the screen to run its action. Hold modifier keys while "
                "choosing an action to make the corner work only while they are held.");
        m_subscription = m_corners.changed.connect([this] { update(); });
        update();
    }

protected:
    void layout() override
    {
        m_unavailableLabel->setColor(Theme::current().disabledText);
        Page::layout();
    }

private:
    void update()
    {
        const bool available = m_corners.isAvailable();
        m_unavailable->setVisible(!available);
        m_list->setVisible(available);
        m_note->setVisible(available);
        for (const auto& [corner, box] : m_boxes) {
            const std::string action = m_corners.action(corner);
            const auto it = std::find(kActions.begin(), kActions.end(), action);
            const int index = it == kActions.end() ? 0 : int(it - kActions.begin());
            box->setCurrentIndex(index);
            const int modifiers = m_corners.modifiers(corner);
            if (modifiers != 0 && index > 0)
                box->setDisplayText(m_corners.modifierText(modifiers) + " – " + kActionNames[size_t(index)]);
            else
                box->setDisplayText(std::nullopt);
        }
    }

    HotCorners m_corners;
    cutefish::base::Subscription m_subscription;
    Card* m_unavailable;
    Label* m_unavailableLabel;
    Card* m_list;
    Footnote* m_note;
    std::vector<std::pair<std::string, RowComboBox*>> m_boxes;
};

} // namespace

static const PageRegistration registration("hotcorners", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<HotCornersPage>(window);
});

} // namespace settings
