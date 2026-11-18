// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/network/common/ipsettingsview.h"

#include <cctype>

#include "base/i18n.h"
#include "cutefish/networkmanager/handler.h"
#include "ui/dialog_window.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;
namespace nm = cutefish::networkmanager;

namespace settings::network {

static constexpr double kFieldWidth = 200;

static int indexOfMethod(const std::vector<nm::ip::Method>& methods, const std::string& value)
{
    for (size_t i = 0; i < methods.size(); ++i) {
        if (methods[i].value == value)
            return int(i);
    }
    return 0;
}

static std::vector<std::string> methodLabels(const std::vector<nm::ip::Method>& methods)
{
    std::vector<std::string> labels;
    for (const nm::ip::Method& method : methods)
        labels.push_back(method.label);
    return labels;
}

static std::string currentValue(const RowComboBox* box, const std::vector<nm::ip::Method>& methods)
{
    const int index = box->currentIndex();
    return index >= 0 && size_t(index) < methods.size() ? methods[size_t(index)].value : std::string();
}

IpSettingsView::IpSettingsView(std::string connectionPath)
    : m_connectionPath(std::move(connectionPath))
    , m_handler(std::make_unique<nm::Handler>())
{
    setSpacing(metrics::dialogSpacing);
    setLayoutHints({.fillWidth = true});

    m_subscriptions.push_back(m_handler->ipSaved.connect([this](const std::string& connection) {
        if (connection != m_connectionPath || !m_saving)
            return;
        m_saving = false;
        if (onFinished)
            onFinished();
    }));
    m_subscriptions.push_back(
        m_handler->ipSaveFailed.connect([this](const std::string& connection, const std::string& message) {
            if (connection != m_connectionPath || !m_saving)
                return;
            m_saving = false;
            m_error = message;
            update();
        }));

    Card* ipv4 = addChild<Card>(Card::Style::List);
    m_ipv4Method = addMethod(ipv4, tr("Configure IPv4"));
    m_ipv4Method->onActivated = [this](int) {
        if (currentValue(m_ipv4Method, m_form.ipv4Methods) == "manual")
            fillFromCurrent(true);
        update();
    };
    m_ipv4AddressRow = addField(ipv4, tr("IP Address"), tr("Required"));
    m_ipv4Address = m_ipv4AddressRow->field();
    m_ipv4NetmaskRow = addField(ipv4, tr("Subnet Mask"), "255.255.255.0");
    m_ipv4Netmask = m_ipv4NetmaskRow->field();
    m_ipv4GatewayRow = addField(ipv4, tr("Router"), tr("Optional"));
    m_ipv4Gateway = m_ipv4GatewayRow->field();

    Card* ipv6 = addChild<Card>(Card::Style::List);
    m_ipv6Method = addMethod(ipv6, tr("Configure IPv6"));
    m_ipv6Method->onActivated = [this](int) {
        if (currentValue(m_ipv6Method, m_form.ipv6Methods) == "manual")
            fillFromCurrent(false);
        update();
    };
    m_ipv6AddressRow = addField(ipv6, tr("IP Address"), tr("Required"));
    m_ipv6Address = m_ipv6AddressRow->field();
    m_ipv6PrefixRow = addField(ipv6, tr("Prefix Length"), "64");
    m_ipv6Prefix = m_ipv6PrefixRow->field();
    m_ipv6GatewayRow = addField(ipv6, tr("Router"), tr("Optional"));
    m_ipv6Gateway = m_ipv6GatewayRow->field();

    Card* dns = addChild<Card>(Card::Style::List);
    m_automaticDns = dns->addChild<SwitchRow>(tr("Automatic DNS"));
    m_automaticDns->onToggled = [this](bool) { update(); };
    m_dns = addField(dns, tr("DNS Servers"), {})->field();

    m_message = addChild<Footnote>();

    RowLayout* buttons = addChild<RowLayout>();
    buttons->setSpacing(metrics::dialogButtonSpacing);
    buttons->setLayoutHints({.horizontalAlignment = Alignment::End});
    m_saveButton = buttons->addChild<DialogButton>(tr("Save"), true);
    m_saveButton->onClicked = [this] { save(); };

    load();
}

IpSettingsView::~IpSettingsView() = default;

RowComboBox* IpSettingsView::addMethod(Card* card, const std::string& text)
{
    ListRow* row = card->addChild<ListRow>(text);
    row->setInteractive(false);
    return row->addContent<RowComboBox>();
}

TextFieldRow* IpSettingsView::addField(Card* card, const std::string& text, const std::string& placeholder)
{
    TextFieldRow* row = card->addChild<TextFieldRow>(text, kFieldWidth);
    row->field()->setPlaceholder(placeholder);
    row->field()->onTextChanged = [this] { update(); };
    row->field()->onAccepted = [this] { save(); };
    return row;
}

void IpSettingsView::load()
{
    m_form = m_handler->ipForm(m_connectionPath);
    m_ipv4Method->setItems(methodLabels(m_form.ipv4Methods));
    m_ipv4Method->setCurrentIndex(indexOfMethod(m_form.ipv4Methods, m_form.ipv4Method));
    m_ipv4Address->setText(m_form.ipv4Address);
    m_ipv4Netmask->setText(m_form.ipv4Netmask);
    m_ipv4Gateway->setText(m_form.ipv4Gateway);
    m_ipv6Method->setItems(methodLabels(m_form.ipv6Methods));
    m_ipv6Method->setCurrentIndex(indexOfMethod(m_form.ipv6Methods, m_form.ipv6Method));
    m_ipv6Address->setText(m_form.ipv6Address);
    m_ipv6Prefix->setText(m_form.ipv6Prefix);
    m_ipv6Gateway->setText(m_form.ipv6Gateway);
    m_automaticDns->setChecked(m_form.automaticDns);
    m_dns->setText(m_form.dns);
    update();
}

nm::ip::Form IpSettingsView::collect() const
{
    nm::ip::Form form;
    form.ipv4Method = currentValue(m_ipv4Method, m_form.ipv4Methods);
    form.ipv4Address = m_ipv4Address->text();
    form.ipv4Netmask = m_ipv4Netmask->text();
    form.ipv4Gateway = m_ipv4Gateway->text();
    form.ipv6Method = currentValue(m_ipv6Method, m_form.ipv6Methods);
    form.ipv6Address = m_ipv6Address->text();
    form.ipv6Prefix = m_ipv6Prefix->text();
    form.ipv6Gateway = m_ipv6Gateway->text();
    form.automaticDns = m_automaticDns->isChecked();
    form.dns = m_dns->text();
    return form;
}

// A manual setup starts from the addresses the connection has now, as they
// usually are what is to be kept.
void IpSettingsView::fillFromCurrent(bool v4)
{
    const nm::ip::Form current = m_handler->currentAddresses(m_connectionPath);
    TextField* address = v4 ? m_ipv4Address : m_ipv6Address;
    const std::string& currentAddress = v4 ? current.ipv4Address : current.ipv6Address;
    if (!address->text().empty() || currentAddress.empty())
        return;
    address->setText(currentAddress);
    (v4 ? m_ipv4Netmask : m_ipv6Prefix)->setText(v4 ? current.ipv4Netmask : current.ipv6Prefix);
    (v4 ? m_ipv4Gateway : m_ipv6Gateway)->setText(v4 ? current.ipv4Gateway : current.ipv6Gateway);
}

void IpSettingsView::save()
{
    if (m_saving || !nm::ip::check(collect()).complete)
        return;
    m_saving = true;
    m_error.clear();
    update();
    m_handler->saveIp(m_connectionPath, collect());
}

void IpSettingsView::update()
{
    const nm::ip::Form form = collect();
    const bool ipv4Manual = form.ipv4Method == "manual";
    const bool ipv6Manual = form.ipv6Method == "manual";
    // Only a family that asks the network for its addresses can be told its name servers too.
    const bool automaticFamily = form.ipv4Method == "auto" || form.ipv4Method == "dhcp" || form.ipv6Method == "auto"
                                 || form.ipv6Method == "dhcp";
    for (TextFieldRow* row : {m_ipv4AddressRow, m_ipv4NetmaskRow, m_ipv4GatewayRow})
        row->setVisible(ipv4Manual);
    for (TextFieldRow* row : {m_ipv6AddressRow, m_ipv6PrefixRow, m_ipv6GatewayRow})
        row->setVisible(ipv6Manual);
    m_automaticDns->setVisible(automaticFamily);
    m_dns->setPlaceholder(automaticFamily && m_automaticDns->isChecked() ? tr("Optional") : "1.1.1.1, 8.8.8.8");

    const nm::ip::Check result = nm::ip::check(form);
    const std::string problem = !m_error.empty() ? m_error : result.message;
    std::string message = problem;
    bool hasDns = false;
    for (const char c : m_dns->text())
        hasDns |= !std::isspace(static_cast<unsigned char>(c));
    if (message.empty() && automaticFamily && m_automaticDns->isChecked() && hasDns)
        message = tr("These servers are asked before the network's own.");
    m_message->setText(message);
    m_message->setColor(problem.empty() ? std::optional<ColorRecipe>() : accent::red);
    m_message->setVisible(!message.empty());

    m_saveButton->setText(m_saving ? tr("Saving…") : tr("Save"));
    m_saveButton->setEnabled(!m_saving && result.complete);
    requestLayout();
}

} // namespace settings::network
