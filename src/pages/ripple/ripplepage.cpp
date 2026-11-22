// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <unistd.h>

#include "base/dbus.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/settings.h"
#include "ui/button.h"
#include "ui/file_chooser.h"
#include "ui/switch.h"
#include "ui/text_field.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::base::Settings;
using cutefish::i18n::format;
using cutefish::i18n::tr;

namespace settings {

namespace {

std::string hostName()
{
    char name[256] = {};
    if (::gethostname(name, sizeof(name) - 1) == 0 && name[0] != '\0')
        return name;
    return "Cutefish";
}

class RipplePage : public Page
{
public:
    explicit RipplePage(SettingsWindow &window)
        : Page(window, tr("Ripple"))
        , m_settings(cutefish::base::paths::cutefishConfig("ripple"))
        , m_service(cutefish::base::dbus::Connection::session(), "org.cutefish.Ripple", "/Ripple",
                    "org.cutefish.Ripple")
    {
        m_name = m_settings.stringValue("Device/Name", hostName());
        m_visible = m_settings.boolValue("Device/Visible", true);
        m_ask = m_settings.boolValue("Receiving/Ask", true);
        m_downloads = m_settings.stringValue(
                "Receiving/Downloads",
                cutefish::base::paths::userDirectory(cutefish::base::paths::UserDirectory::Downloads)
                        + "/Ripple");

        Card *card = content()->addChild<Card>(Card::Style::List);
        TextFieldRow *name = card->addChild<TextFieldRow>(tr("Name"), 220);
        name->setName("ripple:name");
        name->field()->setText(m_name);
        name->field()->onEditingFinished = [this] {
            const std::string text = m_nameField->field()->text();
            if (text.empty() || text == m_name)
                return;
            m_name = text;
            save();
        };
        m_nameField = name;

        SwitchRow *visible = card->addChild<SwitchRow>(tr("Visible to nearby devices"));
        visible->setName("ripple:visible");
        visible->setChecked(m_visible);
        visible->onToggled = [this](bool checked) {
            m_visible = checked;
            save();
        };

        SwitchRow *ask = card->addChild<SwitchRow>(tr("Ask before receiving"));
        ask->setName("ripple:ask");
        ask->setChecked(m_ask);
        ask->onToggled = [this](bool checked) {
            m_ask = checked;
            save();
        };

        ListRow *folder = card->addChild<ListRow>(tr("Save received files to"));
        folder->setInteractive(false);
        Button *choose = folder->addContent<Button>(tr("Choose…"));
        choose->setName("ripple:folder");
        choose->onClicked = [this] { chooseFolder(); };

        m_path = card->addChild<InfoRow>(tr("Folder"), m_downloads);
        m_path->setName("ripple:path");

        Footnote *note = content()->addChild<Footnote>(format(
                tr("Ripple sends files to devices on this network with LocalSend, and receives theirs. "
                   "This computer appears as {name}; names and folders can be changed here and apply at once."),
                {{"name", m_name}}));
        note->setName("ripple:note");
    }

private:
    void chooseFolder()
    {
        m_chooser = std::make_unique<FileChooser>(&settingsWindow().window());
        m_chooser->setMode(FileChooser::Mode::OpenFolder);
        m_chooser->setTitle(tr("Choose where received files go"));
        m_chooser->onAccepted = [this](const std::vector<std::string> &paths, const std::string &) {
            if (paths.empty() || paths.front() == m_downloads)
                return;
            m_downloads = paths.front();
            m_path->setValue(m_downloads);
            save();
        };
        m_chooser->open();
    }

    void save()
    {
        m_settings.setValue("Device/Name", m_name);
        m_settings.setValue("Device/Visible", m_visible);
        m_settings.setValue("Receiving/Ask", m_ask);
        m_settings.setValue("Receiving/Downloads", m_downloads);
        m_settings.sync();
        // The daemon picks the new values up now; a changed folder applies to
        // the next file, and a new name is announced straight away.
        m_service.call("reload");
    }

    Settings m_settings;
    cutefish::base::dbus::Proxy m_service;
    std::string m_name;
    bool m_visible = true;
    bool m_ask = true;
    std::string m_downloads;
    TextFieldRow *m_nameField = nullptr;
    InfoRow *m_path = nullptr;
    std::unique_ptr<FileChooser> m_chooser;
};

} // namespace

static const PageRegistration registration("ripple", [](SettingsWindow &window) -> std::unique_ptr<Page> {
    return std::make_unique<RipplePage>(window);
});

} // namespace settings
