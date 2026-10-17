// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <algorithm>
#include <array>
#include <cstdio>

#include "base/locale.h"
#include "base/paths.h"
#include "base/settings.h"
#include "base/thread_pool.h"
#include "cutefish/appearance/appearance.h"
#include "pages/fonts/fontlist.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish::ui;
using cutefish::appearance::Appearance;

namespace settings {

namespace {

// ThemeManager's fallback size in cutefish-core.
constexpr int kDefaultSize = 9;
constexpr std::array<int, 11> kSizes = {8, 9, 10, 11, 12, 13, 14, 15, 16, 18, 20};
// Indexes are the hinting levels, as fontconfig names them.
constexpr std::array<const char*, 4> kHintStyles = {"hintnone", "hintslight", "hintmedium", "hintfull"};

// The session's language as fontconfig names it, such as "en-US".
std::string languageTag()
{
    std::string name = cutefish::base::Locale::system().name();
    std::replace(name.begin(), name.end(), '_', '-');
    return name;
}

std::string pointSizeText(double size)
{
    char text[32];
    snprintf(text, sizeof(text), "%gpt", size);
    return text;
}

class FontsPage : public Page
{
public:
    explicit FontsPage(SettingsWindow& window)
        : Page(window, "Fonts")
    {
        const cutefish::base::Settings services(cutefish::base::paths::cutefishConfig("services"));
        m_antiAliasing = services.boolValue("Appearance/FontAntialias", true);
        const std::string hintStyle = services.stringValue("Appearance/FontHintStyle", "hintfull");
        const auto hint = std::find(kHintStyles.begin(), kHintStyles.end(), hintStyle);
        m_hinting = hint == kHintStyles.end() ? 0 : int(hint - kHintStyles.begin());

        Card* card = content()->addChild<Card>(Card::Style::List);
        m_general = addRow(card, "General font")->addContent<RowComboBox>();
        m_general->onActivated = [this](int) { m_appearance.setFontFamily(m_general->currentText()); };
        m_fixed = addRow(card, "Fixed font")->addContent<RowComboBox>();
        m_general->setFontPreview(true);
        m_fixed->setFontPreview(true);
        m_fixed->onActivated = [this](int) { m_appearance.setFixedFontFamily(m_fixed->currentText()); };

        std::vector<std::string> sizes;
        for (const int size : kSizes)
            sizes.push_back(size == kDefaultSize ? pointSizeText(size) + " (Default)" : pointSizeText(size));
        m_size = addRow(card, "Font size")->addContent<RowComboBox>(sizes);
        m_size->onActivated = [this](int index) { m_appearance.setFontPointSize(kSizes[size_t(index)]); };

        RowComboBox* hinting = addRow(card, "Hinting")->addContent<RowComboBox>(
                std::vector<std::string>{"None", "Slight", "Medium", "Full"});
        hinting->setCurrentIndex(m_hinting);
        hinting->onActivated = [this](int index) {
            if (index == m_hinting)
                return;
            m_hinting = index;
            save();
        };

        SwitchRow* antiAliasing = card->addChild<SwitchRow>("Anti-Aliasing");
        antiAliasing->setSeparatorVisible(false);
        antiAliasing->setChecked(m_antiAliasing);
        antiAliasing->onToggled = [this](bool checked) {
            m_antiAliasing = checked;
            save();
        };

        m_subscription = m_appearance.changed.connect([this] { update(); });
        update();
        m_scan = cutefish::base::runInThread([language = languageTag()] { return FontList::scan(language); },
                                             [this](FontList fonts) {
                                                 m_fonts = std::move(fonts);
                                                 m_general->setItems(m_fonts.general);
                                                 m_fixed->setItems(m_fonts.fixed);
                                                 update();
                                             });
    }

private:
    static ListRow* addRow(Card* card, const char* text)
    {
        ListRow* row = card->addChild<ListRow>(text);
        row->setInteractive(false);
        row->setSeparatorVisible(false);
        return row;
    }

    static int indexOf(const std::vector<std::string>& names, const std::string& name)
    {
        const auto it = std::find(names.begin(), names.end(), name);
        return it == names.end() ? -1 : int(it - names.begin());
    }

    void update()
    {
        // A family missing from the list leaves the box on the first one.
        if (const int index = indexOf(m_fonts.general, m_appearance.fontFamily()); index >= 0)
            m_general->setCurrentIndex(index);
        if (const int index = indexOf(m_fonts.fixed, m_appearance.fixedFontFamily()); index >= 0)
            m_fixed->setCurrentIndex(index);
        // The menu marks the default; the box shows only the size.
        const double size = m_appearance.fontPointSize();
        const auto it = std::find(kSizes.begin(), kSizes.end(), size);
        m_size->setCurrentIndex(it == kSizes.end() ? -1 : int(it - kSizes.begin()));
        m_size->setDisplayText(pointSizeText(size));
    }

    void save() { m_appearance.setFontRendering(m_antiAliasing, kHintStyles[size_t(m_hinting)]); }

    Appearance m_appearance;
    cutefish::base::Subscription m_subscription;
    cutefish::base::Job m_scan;
    FontList m_fonts;
    RowComboBox* m_general;
    RowComboBox* m_fixed;
    RowComboBox* m_size;
    bool m_antiAliasing = true;
    int m_hinting = 3;
};

} // namespace

static const PageRegistration registration("fonts", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<FontsPage>(window);
});

} // namespace settings
