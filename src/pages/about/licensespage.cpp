// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "base/i18n.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {

namespace {

class LicensesPage : public Page
{
public:
    explicit LicensesPage(SettingsWindow& window)
        : Page(window, tr("Open-Source Licenses"))
    {
        Section* cutefish = content()->addChild<Section>(tr("Cutefish"));
        Card* cutefishCard = cutefish->addChild<Card>(Card::Style::List);
        cutefishCard->addChild<InfoRow>(tr("Desktop and applications"), "BSD-3-Clause");
        cutefish->addChild<Footnote>(tr("Terminal and the icon theme are under the GNU General Public License 3.0."));

        Section* compositor = content()->addChild<Section>(tr("Compositor"));
        Card* compositorCard = compositor->addChild<Card>(Card::Style::List);
        compositorCard->addChild<InfoRow>(tr("Wayland and window management, designed after Chromium's Exo and Ash"),
                                          "BSD-3-Clause");
        compositorCard->addChild<InfoRow>(tr("Xwayland window manager and selection bridge, adapted from sommelier"),
                                          "BSD-3-Clause");
        compositorCard->addChild<InfoRow>(tr("Pointer acceleration curve"), "APSL-2.0");
        // Apple Public Source License 2.0, section 2.3: the executable carries a notice of where its source is.
        compositor->addChild<Footnote>(
                tr("The source code of the pointer acceleration curve is available under the Apple Public Source "
                   "License 2.0 at https://github.com/cutefishos/cutefish-compositor."));
    }
};

} // namespace

static const PageRegistration registration("licenses", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<LicensesPage>(window);
});

} // namespace settings
