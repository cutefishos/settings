// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/network/wlan/newnetworkdialog.h"

#include <algorithm>
#include <cctype>

#include "base/i18n.h"
#include "ui/dialog_window.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/window.h"
#include "widgets/card.h"
#include "widgets/formdialog.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"

using namespace cutefish::ui;
using cutefish::i18n::format;
using cutefish::i18n::tr;
using cutefish::i18n::trNoop;
namespace nm = cutefish::networkmanager;

namespace settings::network {

static constexpr double kWidth = 400;
static constexpr double kFieldWidth = 200;

struct SecurityChoice
{
    const char* text;
    const char* value;
};

static const SecurityChoice kSecurities[] = {
    // TRANSLATORS: a Wi-Fi network's security: none, anyone can join.
    {trNoop("Open"), "none"},
    {"WEP", "wep"},
    {"WPA", "wpa"},
    {"WPA2/WPA3", "wpa2-wpa3"},
    {"WPA3", "wpa3"},
    {trNoop("WPA Enterprise"), "wpa-eap"},
    {trNoop("WPA2 Enterprise"), "wpa2-eap"},
    {trNoop("WPA3 Enterprise"), "wpa3-eap"},
};
static constexpr int kDefaultSecurity = 3;

static int indexOfSecurity(const std::string& value)
{
    for (size_t i = 0; i < std::size(kSecurities); ++i) {
        if (value == kSecurities[i].value)
            return int(i);
    }
    return kDefaultSecurity;
}

NewNetworkDialog::NewNetworkDialog(SettingsWindow& window)
    : m_window(window)
{
}

NewNetworkDialog::~NewNetworkDialog() = default;

void NewNetworkDialog::openNetwork(const std::string& accessPoint, const std::optional<nm::WirelessNetwork>& network)
{
    if (!network || network->device.empty())
        return;

    m_accessPoint = accessPoint;
    m_device = network->device;
    m_networkName = network->name;
    m_needsCertificate = network->security == nm::SecurityType::Wpa3SuiteB192;
    build();
    m_security->setCurrentIndex(indexOfSecurity(network->security == nm::SecurityType::WpaEap    ? "wpa-eap"
                                                : network->security == nm::SecurityType::Wpa2Eap ? "wpa2-eap"
                                                                                                 : "wpa3-eap"));
    update();
    m_dialog->show();
    m_userName->setFocus();
}

void NewNetworkDialog::show()
{
    if (isOpen())
        return;
    m_accessPoint.clear();
    m_device.clear();
    m_networkName.clear();
    m_needsCertificate = false;
    build();
    update();
    m_dialog->show();
    m_name->setFocus();
}

void NewNetworkDialog::close()
{
    if (m_dialog)
        m_dialog->close();
}

bool NewNetworkDialog::isOpen() const
{
    return m_dialog && m_dialog->isOpen();
}

std::string NewNetworkDialog::title() const
{
    return m_accessPoint.empty() ? std::string(tr("Add Other Network"))
                                 : format(tr("Join “{network}”"), {{"network", m_networkName}});
}

std::string NewNetworkDialog::security() const
{
    const int index = m_security ? m_security->currentIndex() : -1;
    return index >= 0 && size_t(index) < std::size(kSecurities) ? kSecurities[index].value : "";
}

static bool isHex(const std::string& text)
{
    return std::all_of(text.begin(), text.end(), [](char c) { return std::isxdigit(static_cast<unsigned char>(c)); });
}

// What NetworkManager accepts for each kind of key: a passphrase, or the key
// itself in hexadecimal.
bool NewNetworkDialog::passwordValid(const std::string& password) const
{
    const std::string type = security();
    if (type == "none")
        return true;
    const size_t length = password.size();
    if (type == "wep")
        return length == 5 || length == 13 || ((length == 10 || length == 26) && isHex(password));
    if (type == "wpa" || type == "wpa2-wpa3" || type == "wpa3")
        return (length >= 8 && length <= 63) || (length == 64 && isHex(password));
    return !password.empty();
}

bool NewNetworkDialog::canJoin() const
{
    if (!m_dialog)
        return false;
    const bool enterprise = security().ends_with("-eap");
    return !m_needsCertificate && (!m_accessPoint.empty() || !m_name->text().empty())
           && (!enterprise || !m_userName->text().empty()) && passwordValid(m_password->text());
}

void NewNetworkDialog::join()
{
    if (!canJoin())
        return;

    if (!m_accessPoint.empty()) {
        if (onJoinNetwork)
            onJoinNetwork(m_device, m_accessPoint, m_userName->text(), m_password->text());
    } else if (onConnect) {
        const std::string type = security();
        onConnect(m_name->text(), type.ends_with("-eap") ? m_userName->text() : std::string(),
                  type == "none" ? std::string() : m_password->text(), type);
    }
    close();
}

void NewNetworkDialog::build()
{
    m_dialog = std::make_unique<FormDialog>(m_window, title(), kWidth);
    m_dialog->onClosed = [this] {
        // Closing returns the dialog to a hidden network.
        m_accessPoint.clear();
        m_device.clear();
        m_networkName.clear();
        m_needsCertificate = false;
    };
    ColumnLayout* content = m_dialog->content();

    m_form = content->addChild<Card>(Card::Style::List);
    const auto addField = [this](const char* label) {
        TextFieldRow* row = m_form->addChild<TextFieldRow>(label, kFieldWidth);
        row->field()->setPlaceholder(tr("Required"));
        row->field()->onTextChanged = [this] { update(); };
        row->field()->onAccepted = [this] { join(); };
        return row;
    };

    m_nameRow = addField(tr("Network name"));
    m_name = m_nameRow->field();
    m_securityRow = m_form->addChild<ListRow>(tr("Security"));
    m_securityRow->setInteractive(false);
    std::vector<std::string> securities;
    for (const SecurityChoice& choice : kSecurities)
        securities.push_back(tr(choice.text));
    m_security = m_securityRow->addContent<RowComboBox>(securities);
    m_security->setCurrentIndex(kDefaultSecurity);
    m_security->onActivated = [this](int) { update(); };
    m_userNameRow = addField(tr("Username"));
    m_userName = m_userNameRow->field();
    m_passwordRow = addField(tr("Password"));
    m_password = m_passwordRow->field();
    m_password->setEchoMode(TextField::EchoMode::Password);

    m_certificateNote = content->addChild<Footnote>(
            tr("This network signs in with a certificate, which can't be set up here yet."));

    m_joinButton = m_dialog->addButtonRow()->addChild<DialogButton>(tr("Join"), true);
    m_joinButton->onClicked = [this] { join(); };
}

void NewNetworkDialog::update()
{
    if (!m_dialog)
        return;
    const std::string type = security();
    const bool listed = !m_accessPoint.empty();
    const bool enterprise = type.ends_with("-eap");
    const bool userName = enterprise && !m_needsCertificate;
    const bool password = type != "none" && !m_needsCertificate;
    m_dialog->setTitle(title());
    m_nameRow->setVisible(!listed);
    m_securityRow->setVisible(!listed);
    m_userNameRow->setVisible(userName);
    m_passwordRow->setVisible(password);
    m_form->setVisible(!listed || userName || password);
    m_certificateNote->setVisible(m_needsCertificate);
    m_joinButton->setEnabled(canJoin());
}

} // namespace settings::network
