// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "base/i18n.h"
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
using cutefish::i18n::format;
using cutefish::i18n::tr;

namespace settings {

namespace {

class AboutPage : public Page
{
public:
    explicit AboutPage(SettingsWindow& window)
        : Page(window, tr("About"))
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
        m_runningOn = brand->addChild<Label>(
                format(tr("Running on {system}"), {{"system", about::prettyProductName()}}));
        m_runningOn->setLayoutHints({.horizontalAlignment = Alignment::Center, .verticalAlignment = Alignment::Start});

        Card* card = content()->addChild<Card>(Card::Style::List);
        const std::string model = about::computerModel();
        if (!model.empty())
            card->addChild<InfoRow>(tr("Model"), model);
        card->addChild<InfoRow>(tr("System version"), about::prettyProductName());
        card->addChild<InfoRow>(tr("System type"), about::architecture());
        card->addChild<InfoRow>(tr("Kernel version"), about::kernelVersion());
        card->addChild<InfoRow>(tr("Processor"), about::processor());
        card->addChild<InfoRow>(tr("RAM"), about::memorySize());
        card->addChild<InfoRow>(tr("Internal storage"), about::internalStorage());
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
