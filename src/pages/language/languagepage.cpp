// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "base/event_loop.h"
#include "pages/language/addlanguagedialog.h"
#include "pages/language/language.h"
#include "pages/language/preferredlanguagelist.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "widgets/card.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish;
using namespace cutefish::ui;

namespace settings {

namespace {

class LanguagePage : public Page
{
public:
    explicit LanguagePage(SettingsWindow& window)
        : Page(window, "Language")
    {
        Section* section = content()->addChild<Section>("Preferred languages");
        Card* card = section->addChild<Card>(Card::Style::Padded);
        card->setPadding({metrics::cardHorizontalPadding, metrics::xs, metrics::cardHorizontalPadding, metrics::xs});
        card->addChild<PreferredLanguageList>(m_language);
        section->addChild<Footnote>(
                "Drag a language to the top to make it the system language. Right-click a language to remove it.");

        m_add = content()->addChild<StandardButton>("Add language…");
        m_add->onClicked = [this] {
            m_dialog = std::make_unique<AddLanguageDialog>(settingsWindow(), m_language);
            m_dialog->onClosed = [this] {
                std::shared_ptr<AddLanguageDialog> closing(m_dialog.release());
                base::EventLoop::current()->post([closing] {});
            };
            m_dialog->show();
        };

        m_error = content()->addChild<Label>();
        m_error->setWrap(true);
        m_error->setLayoutHints({.fillWidth = true});

        m_subscriptions.push_back(m_language.savingChanged.connect([this] { update(); }));
        m_subscriptions.push_back(m_language.errorMessageChanged.connect([this] { update(); }));
        update();
    }

private:
    void update()
    {
        m_add->setEnabled(!m_language.isSaving());
        m_error->setText(m_language.errorMessage());
        m_error->setVisible(!m_language.errorMessage().empty());
    }

    Language m_language;
    std::vector<base::Subscription> m_subscriptions;
    StandardButton* m_add;
    Label* m_error;
    std::unique_ptr<AddLanguageDialog> m_dialog;
};

} // namespace

static const PageRegistration registration("language", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<LanguagePage>(window);
});

} // namespace settings
