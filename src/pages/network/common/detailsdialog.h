// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/signal.h"
#include "cutefish/networkmanager/network_model.h"

namespace cutefish::networkmanager {
class ConnectionSettings;
class Handler;
}

namespace cutefish::ui {
class ColumnLayout;
class DialogButton;
class DialogWindow;
}

namespace settings {
class InfoRow;
class SettingsWindow;
class SwitchRow;
}

namespace settings::network {

class IpSettingsView;
class NetworkList;

// A Wi-Fi network's or a wired profile's details: whether it connects by
// itself, its addresses, and the way to its IP settings, which take the
// details' place with a way back in the title bar. The network is followed
// across the list's changes; the dialog closes once it is gone.
class DetailsDialog
{
public:
    enum class Kind {
        Wireless,
        Wired,
    };

    // A saved network is followed by its connection, others by name.
    DetailsDialog(SettingsWindow& window, NetworkList& list, Kind kind,
                  const cutefish::networkmanager::NetworkItem& item);
    ~DetailsDialog();

    DetailsDialog(const DetailsDialog&) = delete;
    DetailsDialog& operator=(const DetailsDialog&) = delete;

    void show();
    cutefish::ui::DialogWindow* dialog() const { return m_dialog.get(); }

    std::function<void()> onClosed;

private:
    class Content;

    std::optional<cutefish::networkmanager::NetworkItem> find() const;
    void update();
    void showIpSettings(bool shown);
    void setSaved(bool saved);
    void close();

    SettingsWindow& m_window;
    NetworkList& m_list;
    Kind m_kind;
    std::string m_connectionPath;
    std::string m_ssid;
    std::unique_ptr<cutefish::networkmanager::Handler> m_handler;
    std::unique_ptr<cutefish::networkmanager::ConnectionSettings> m_settings;
    std::unique_ptr<cutefish::ui::DialogWindow> m_dialog;
    cutefish::base::Subscription m_subscription;
    cutefish::networkmanager::NetworkItem m_row;
    bool m_found = false;
    bool m_saved = false;
    bool m_ipSettingsShown = false;

    Content* m_content = nullptr;
    cutefish::ui::ColumnLayout* m_details = nullptr;
    cutefish::ui::ColumnLayout* m_savedCard = nullptr;
    SwitchRow* m_autoConnect = nullptr;
    InfoRow* m_security = nullptr;
    InfoRow* m_signal = nullptr;
    cutefish::ui::ColumnLayout* m_addressCard = nullptr;
    InfoRow* m_ipv4 = nullptr;
    InfoRow* m_ipv6 = nullptr;
    InfoRow* m_mac = nullptr;
    InfoRow* m_gateway = nullptr;
    InfoRow* m_dns = nullptr;
    cutefish::ui::DialogButton* m_forget = nullptr;
    IpSettingsView* m_ipSettings = nullptr;
};

} // namespace settings::network
