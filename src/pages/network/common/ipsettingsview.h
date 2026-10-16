// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "base/signal.h"
#include "networkmanager/iputils.h"
#include "ui/cell_layout.h"

namespace cutefish::networkmanager {
class Handler;
}

namespace cutefish::ui {
class ComboBox;
class DialogButton;
class Label;
class Switch;
class TextField;
}

namespace settings::network {

// How a saved network gets its addresses and name servers, shown in place of
// its details. Save writes the profile and applies it to the connection at
// once; onFinished follows a save or Cancel.
class IpSettingsView : public cutefish::ui::ColumnLayout
{
public:
    explicit IpSettingsView(std::string connectionPath);
    ~IpSettingsView() override;

    std::function<void()> onFinished;

protected:
    void layout() override;

private:
    struct FormRow
    {
        cutefish::ui::RowLayout* row;
        cutefish::ui::Label* label;
    };

    FormRow addRow(cutefish::ui::ColumnLayout* card, const std::string& label);
    cutefish::ui::TextField* addField(cutefish::ui::RowLayout* row, const std::string& placeholder);
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
    std::vector<cutefish::ui::Label*> m_fieldLabels;

    cutefish::ui::ComboBox* m_ipv4Method;
    cutefish::ui::RowLayout* m_ipv4AddressRow;
    cutefish::ui::TextField* m_ipv4Address;
    cutefish::ui::RowLayout* m_ipv4NetmaskRow;
    cutefish::ui::TextField* m_ipv4Netmask;
    cutefish::ui::RowLayout* m_ipv4GatewayRow;
    cutefish::ui::TextField* m_ipv4Gateway;
    cutefish::ui::ComboBox* m_ipv6Method;
    cutefish::ui::RowLayout* m_ipv6AddressRow;
    cutefish::ui::TextField* m_ipv6Address;
    cutefish::ui::RowLayout* m_ipv6PrefixRow;
    cutefish::ui::TextField* m_ipv6Prefix;
    cutefish::ui::RowLayout* m_ipv6GatewayRow;
    cutefish::ui::TextField* m_ipv6Gateway;
    cutefish::ui::RowLayout* m_automaticDnsRow;
    cutefish::ui::Switch* m_automaticDns;
    cutefish::ui::TextField* m_dns;
    cutefish::ui::Label* m_message;
    bool m_problem = false;
    cutefish::ui::DialogButton* m_saveButton;
};

} // namespace settings::network
