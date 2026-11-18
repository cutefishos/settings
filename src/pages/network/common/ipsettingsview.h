// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "cutefish/networkmanager/ip_utils.h"
#include "ui/cell_layout.h"

namespace cutefish::networkmanager {
class Handler;
}

namespace cutefish::ui {
class DialogButton;
class TextField;
}

namespace settings {
class Card;
class Footnote;
class RowComboBox;
class SwitchRow;
class TextFieldRow;
}

namespace settings::network {

// How a saved network gets its addresses and name servers, shown in place of
// its details. Save writes the profile and applies it to the connection at
// once; onFinished follows a save.
class IpSettingsView : public cutefish::ui::ColumnLayout
{
public:
    explicit IpSettingsView(std::string connectionPath);
    ~IpSettingsView() override;

    std::function<void()> onFinished;

private:
    RowComboBox* addMethod(Card* card, const std::string& text);
    TextFieldRow* addField(Card* card, const std::string& text, const std::string& placeholder);
    void load();
    cutefish::networkmanager::ip::Form collect() const;
    void fillFromCurrent(bool v4);
    void save();
    void update();

    std::string m_connectionPath;
    std::unique_ptr<cutefish::networkmanager::Handler> m_handler;
    std::vector<cutefish::base::Subscription> m_subscriptions;
    cutefish::networkmanager::ip::Form m_form;
    bool m_saving = false;
    std::string m_error;

    RowComboBox* m_ipv4Method;
    TextFieldRow* m_ipv4AddressRow;
    cutefish::ui::TextField* m_ipv4Address;
    TextFieldRow* m_ipv4NetmaskRow;
    cutefish::ui::TextField* m_ipv4Netmask;
    TextFieldRow* m_ipv4GatewayRow;
    cutefish::ui::TextField* m_ipv4Gateway;
    RowComboBox* m_ipv6Method;
    TextFieldRow* m_ipv6AddressRow;
    cutefish::ui::TextField* m_ipv6Address;
    TextFieldRow* m_ipv6PrefixRow;
    cutefish::ui::TextField* m_ipv6Prefix;
    TextFieldRow* m_ipv6GatewayRow;
    cutefish::ui::TextField* m_ipv6Gateway;
    SwitchRow* m_automaticDns;
    cutefish::ui::TextField* m_dns;
    Footnote* m_message;
    cutefish::ui::DialogButton* m_saveButton;
};

} // namespace settings::network
