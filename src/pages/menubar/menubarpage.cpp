// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <string>

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
        leading->addChild<RowIcon>(std::move(glyph), tint);
        onClicked = [this] {
            m_checkBox->setChecked(!m_checkBox->isChecked());
            if (onToggled)
                onToggled(m_checkBox->isChecked());
        };
    }

    void setChecked(bool checked) { m_checkBox->setChecked(checked); }
    bool isChecked() const { return m_checkBox->isChecked(); }

    std::function<void(bool checked)> onToggled;

private:
    CheckBox* m_checkBox;
};

class MenuBarPage : public Page
{
public:
    explicit MenuBarPage(SettingsWindow& window)
        : Page(window, tr("Menu Bar"))
    {
        Section* controls = content()->addChild<Section>(tr("Menu Bar Controls"));
        Card* list = controls->addChild<Card>(Card::Style::List);
        addControl(list, MenuBarSettings::kWiFi, tr("Wi-Fi"), "network-wireless-connected-100", Color::rgb(0x0067FF));
        addControl(list, MenuBarSettings::kBluetooth, tr("Bluetooth"), "bluetooth.svg", Color::rgb(0x0067FF));
        addControl(list, MenuBarSettings::kBattery, tr("Battery"), "battery.svg", Color::rgb(0x2EC347));
        addControl(list, MenuBarSettings::kSound, tr("Sound"), "sound.svg", Color::rgb(0xF16884));
    }

private:
    void addControl(Card* list, const char* control, std::string text, std::string glyph, Color tint)
    {
        ControlRow* row = list->addChild<ControlRow>(std::move(text), std::move(glyph), tint);
        row->setName(std::string("menubar:") + control);
        row->setChecked(m_settings.isControlShown(control));
        row->onToggled = [this, control](bool checked) { m_settings.setControlShown(control, checked); };
    }

    MenuBarSettings m_settings;
};

} // namespace

static const PageRegistration registration("menubar", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<MenuBarPage>(window);
});

} // namespace settings
