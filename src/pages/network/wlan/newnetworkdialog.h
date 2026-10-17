// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "cutefish/networkmanager/handler.h"

namespace cutefish::ui {
class ComboBox;
class DialogButton;
class GridLayout;
class Label;
class TextField;
}

namespace settings {
class FormDialog;
class SettingsWindow;
}

namespace settings::network {

// Joins a network that doesn't broadcast its name, or signs in to an
// enterprise network in range with a user name and password.
class NewNetworkDialog
{
public:
    // A hidden network, by its name.
    explicit NewNetworkDialog(SettingsWindow& window);
    ~NewNetworkDialog();

    // An enterprise network in range; shows nothing once the access point is gone.
    void openNetwork(const std::string& accessPoint,
                     const std::optional<cutefish::networkmanager::WirelessNetwork>& network);
    void show();
    void close();
    bool isOpen() const;

    std::string title() const;
    // "none", "wep", "wpa", "wpa2-wpa3", "wpa3", "wpa-eap", "wpa2-eap" or "wpa3-eap".
    std::string security() const;
    bool needsCertificate() const { return m_needsCertificate; }
    bool canJoin() const;
    void join();

    cutefish::ui::TextField* nameField() const { return m_name; }
    cutefish::ui::ComboBox* securityBox() const { return m_security; }
    cutefish::ui::TextField* userNameField() const { return m_userName; }
    cutefish::ui::TextField* passwordField() const { return m_password; }
    FormDialog* dialog() const { return m_dialog.get(); }

    // Joining a hidden network: its name, the user name of an enterprise
    // one, the password (empty for an open one) and the security.
    std::function<void(const std::string& ssid, const std::string& userName, const std::string& password,
                       const std::string& type)>
        onConnect;
    // Signing in to a network in range.
    std::function<void(const std::string& device, const std::string& accessPoint, const std::string& userName,
                       const std::string& password)>
        onJoinNetwork;

private:
    void build();
    bool passwordValid(const std::string& password) const;
    void update();

    SettingsWindow& m_window;
    std::unique_ptr<FormDialog> m_dialog;
    std::string m_accessPoint;
    std::string m_device;
    std::string m_networkName;
    // 192-bit networks sign in with EAP-TLS, which needs certificates no field asks for.
    bool m_needsCertificate = false;

    cutefish::ui::GridLayout* m_form = nullptr;
    std::vector<cutefish::ui::Label*> m_labels;
    cutefish::ui::Label* m_nameLabel = nullptr;
    cutefish::ui::TextField* m_name = nullptr;
    cutefish::ui::Label* m_securityLabel = nullptr;
    cutefish::ui::ComboBox* m_security = nullptr;
    cutefish::ui::Label* m_userNameLabel = nullptr;
    cutefish::ui::TextField* m_userName = nullptr;
    cutefish::ui::Label* m_passwordLabel = nullptr;
    cutefish::ui::TextField* m_password = nullptr;
    cutefish::ui::Label* m_certificateNote = nullptr;
    cutefish::ui::DialogButton* m_joinButton = nullptr;
};

} // namespace settings::network
