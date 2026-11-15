// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <memory>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "pages/menubar/menubarsettings.h"
#include "ui/check_box.h"
#include "ui/layout.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/rowicon.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {

namespace {

// A check box, the control's tile and its name; a click anywhere on the row
// turns the control on or off.
class ControlRow : public ListRow
{
public:
    ControlRow(std::string text, std::string glyph, Color tint)
        : ListRow(std::move(text))
    {
        RowLayout* leading = setLeading<RowLayout>();
        leading->setSpacing(12);
        m_checkBox = leading->addChild<CheckBox>();
        m_checkBox->setPadding({});
        m_checkBox->onToggled = [this](bool checked) {
            if (onToggled)
                onToggled(checked);
        };
        m_icon = leading->addChild<RowIcon>(std::move(glyph), tint);
        onClicked = [this] {
            m_checkBox->setChecked(!m_checkBox->isChecked());
            if (onToggled)
                onToggled(m_checkBox->isChecked());
        };
    }

    void setChecked(bool checked) { m_checkBox->setChecked(checked); }
    bool isChecked() const { return m_checkBox->isChecked(); }

    std::function<void(bool checked)> onToggled;

protected:
    void enabledChanged() override
    {
        m_icon->setOpacity(isEnabled() ? 1 : 0.4);
        ListRow::enabledChanged();
    }

private:
    CheckBox* m_checkBox;
    RowIcon* m_icon;
};

class MenuBarPage : public Page
{
public:
    explicit MenuBarPage(SettingsWindow& window)
        : Page(window, tr("Menu Bar"))
    {
        Section* controls = content()->addChild<Section>(tr("Menu Bar Controls"));
        Card* list = controls->addChild<Card>(Card::Style::List);
        addControl(window, list, MenuBarSettings::kWiFi, "wlan");
        addControl(window, list, MenuBarSettings::kBluetooth, "bluetooth");
        addControl(window, list, MenuBarSettings::kBattery, "battery");
        addControl(window, list, MenuBarSettings::kSound, "sound");
    }

private:
    void addControl(SettingsWindow& window, Card* list, const char* control, const std::string& pageId)
    {
        const PageInfo* page = findPage(pageId);
        if (!page)
            return;
        ControlRow* row = list->addChild<ControlRow>(tr(page->title.c_str()), page->icon, page->iconColor);
        row->setName(std::string("menubar:") + control);
        row->setChecked(m_settings.isControlShown(control));
        row->onToggled = [this, control](bool checked) { m_settings.setControlShown(control, checked); };
        if (!page->watchAvailability)
            return;
        row->setEnabled(page->initiallyAvailable);
        m_watches.push_back(page->watchAvailability(window, [row](bool available) { row->setEnabled(available); }));
    }

    MenuBarSettings m_settings;
    std::vector<std::unique_ptr<AvailabilityWatch>> m_watches;
};

} // namespace

static const PageRegistration registration("menubar", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<MenuBarPage>(window);
});

} // namespace settings
