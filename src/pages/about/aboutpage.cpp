// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <cmath>

#include "base/i18n.h"
#include "pages/about/aboutinfo.h"
#include "pages/about/devicename.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::i18n::format;
using cutefish::i18n::tr;

namespace settings {

// Bluetooth names are at most 248 bytes.
static constexpr size_t kNameLength = 248;

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
        ListRow* nameRow = card->addChild<ListRow>(tr("Device name"));
        nameRow->setInteractive(false);
        m_nameRow = nameRow;
        m_name = nameRow->addContent<TextField>();
        m_name->setName("about:name");
        m_name->setFrameVisible(false);
        m_name->setRightAligned(true);
        m_name->setTextMargins(0, 0);
        m_name->setText(m_deviceName.name());
        m_name->onTextChanged = [this] {
            if (m_name->text().size() > kNameLength) {
                std::string text = m_name->text();
                // Not inside a character.
                size_t end = kNameLength;
                while (end > 0 && (static_cast<unsigned char>(text[end]) & 0xc0) == 0x80)
                    --end;
                text.resize(end);
                m_name->setText(text);
            }
        };
        // Return keeps the field focused and selects the whole name; this
        // comes before onEditingFinished.
        m_name->onAccepted = [this] { m_selectOnFinish = true; };
        m_name->onEditingFinished = [this] {
            if (m_name->text().empty())
                m_name->setText(m_deviceName.name());
            else
                m_deviceName.setName(m_name->text());
            if (m_selectOnFinish) {
                m_selectOnFinish = false;
                m_name->selectAll();
            }
        };
        m_nameChanged = m_deviceName.changed.connect([this] {
            // Setting the same text again would drop the selection.
            if (m_name->text() != m_deviceName.name())
                m_name->setText(m_deviceName.name());
        });
        const std::string model = about::computerModel();
        if (!model.empty())
            card->addChild<InfoRow>(tr("Model"), model);
        card->addChild<InfoRow>(tr("System version"), about::prettyProductName());
        card->addChild<InfoRow>(tr("System type"), about::architecture());
        card->addChild<InfoRow>(tr("Kernel version"), about::kernelVersion());
        card->addChild<InfoRow>(tr("Processor"), about::processor());
        card->addChild<InfoRow>(tr("RAM"), about::memorySize());
        card->addChild<InfoRow>(tr("Internal storage"), about::internalStorage());

        Card* licensesCard = content()->addChild<Card>(Card::Style::List);
        NavigationItem* licenses = licensesCard->addChild<NavigationItem>(tr("Open-Source Licenses"));
        licenses->onClicked = [&window] { window.pushPage("licenses"); };
    }

protected:
    void layout() override
    {
        m_runningOn->setColor(Theme::current().disabledText);
        // As wide as an InfoRow's value may grow, and as tall as a row
        // leaves, so the name sits where the values do; a right-aligned field
        // keeps 1.5 px past its text for the caret.
        const double cap = std::round(m_nameRow->geometry().width * 0.65);
        if (m_name->layoutHints().maximumWidth != cap) {
            LayoutHints hints{.fillWidth = true,
                              .preferredHeight = metrics::rowHeight - metrics::cardVerticalPadding,
                              .maximumWidth = cap};
            hints.margins.right = -1.5;
            m_name->setLayoutHints(hints);
        }
        Page::layout();
    }

private:
    DeviceName m_deviceName;
    Label* m_runningOn;
    ListRow* m_nameRow;
    TextField* m_name;
    bool m_selectOnFinish = false;
    cutefish::base::Subscription m_nameChanged;
};

} // namespace

static const PageRegistration registration("about", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<AboutPage>(window);
});

} // namespace settings
