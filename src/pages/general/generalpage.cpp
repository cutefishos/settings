// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "base/i18n.h"
#include "base/signal.h"
#include "ui/cell_layout.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowicon.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings::general {

namespace {

// The system-wide pages, in cards of what belongs together: the machine and
// its updates, the clock and the language, the default applications.
class GeneralPage : public Page
{
public:
    explicit GeneralPage(SettingsWindow& window)
        : Page(window, tr("General"))
    {
        const std::vector<std::vector<std::string>> groups = {
            {"about", "softwareupdate"},
            {"datetime", "language"},
            {"defaultapps"},
        };
        for (const std::vector<std::string>& group : groups) {
            Card* card = content()->addChild<Card>(Card::Style::List);
            for (const std::string& page : group) {
                const PageInfo* info = findPage(page);
                NavigationItem* item = card->addChild<NavigationItem>(tr(info->title.c_str()));
                item->setName("general:" + page);
                item->setLeading<RowIcon>(info->icon, info->iconColor);
                item->onClicked = [&window, page] { window.pushPage(page); };
                m_items.emplace_back(page, item);
            }
        }

        m_subscription = window.subPageAvailabilityChanged.connect([this] { update(); });
        update();
    }

private:
    void update()
    {
        for (const auto& [page, item] : m_items)
            item->setVisible(settingsWindow().isSubPageAvailable(page));
    }

    std::vector<std::pair<std::string, NavigationItem*>> m_items;
    cutefish::base::Subscription m_subscription;
};

} // namespace

static const PageRegistration registration("general", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<GeneralPage>(window);
});

} // namespace settings::general
