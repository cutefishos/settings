// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/about/aboutinfo.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish::ui;

namespace settings {

namespace {

class AboutPage : public Page
{
public:
    explicit AboutPage(SettingsWindow& window)
        : Page(window, "About")
    {
        // A nested layout never grows past its children's width, so it is
        // centred itself rather than filled.
        ColumnLayout* brand = content()->addChild<ColumnLayout>();
        brand->setSpacing(metrics::s);
        LayoutHints brandHints{.horizontalAlignment = Alignment::Center};
        brandHints.margins.top = metrics::l;
        brand->setLayoutHints(brandHints);
        Picture* logo = brand->addChild<Picture>(dataPath("images/logo.svg"));
        logo->setLayoutHints({.preferredWidth = 140, .preferredHeight = 72,
                              .horizontalAlignment = Alignment::Center, .verticalAlignment = Alignment::Start});
        Label* name = brand->addChild<Label>("Cutefish");
        name->setFontSize(metrics::points(22));
        name->setWeight(700);
        name->setColor(Color::rgb(0x3385FF));
        name->setLayoutHints({.horizontalAlignment = Alignment::Center, .verticalAlignment = Alignment::Start});
        m_runningOn = brand->addChild<Label>("Running on " + about::prettyProductName());
        m_runningOn->setLayoutHints({.horizontalAlignment = Alignment::Center, .verticalAlignment = Alignment::Start});

        Card* card = content()->addChild<Card>(Card::Style::List);
        card->addChild<InfoRow>("System version", about::prettyProductName());
        card->addChild<InfoRow>("System type", about::architecture());
        card->addChild<InfoRow>("Kernel version", about::kernelVersion());
        card->addChild<InfoRow>("Processor", about::processor());
        card->addChild<InfoRow>("RAM", about::memorySize());
        card->addChild<InfoRow>("Internal storage", about::internalStorage());
    }

protected:
    void layout() override
    {
        m_runningOn->setColor(Theme::current().disabledText);
        Page::layout();
    }

private:
    Label* m_runningOn;
};

} // namespace

static const PageRegistration registration("about", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<AboutPage>(window);
});

} // namespace settings
