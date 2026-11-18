// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cutefish/networkmanager/handler.h"
#include "ui/cell_layout.h"

namespace cutefish::ui {
class TextField;
}

namespace settings {
class Card;
class Footnote;
class SwitchRow;
}

namespace settings::network {

class PluginForm;
class WireGuardEditor;

// What a VPN profile is made of, for the Add VPN and details dialogs: its
// name, the fields of its kind, and who may use it. load() takes
// Handler::vpnForm(), collect() gives Handler::saveVpn() its form.
class VpnEditor : public cutefish::ui::ColumnLayout
{
public:
    VpnEditor();
    ~VpnEditor() override;

    void load(const cutefish::networkmanager::VpnForm& form);
    void setSecrets(const cutefish::networkmanager::VpnSecrets& secrets);
    cutefish::networkmanager::VpnValues collect() const;
    void focusFirst();

    const cutefish::networkmanager::VpnForm& form() const { return m_form; }
    bool installed() const { return m_form.installed; }
    bool complete() const;

    // Return in a field.
    std::function<void()> onAccepted;
    // Anything that changes complete() or the size.
    std::function<void()> onChanged;

private:
    void changed();

    cutefish::networkmanager::VpnForm m_form;
    Footnote* m_notInstalled;
    // The name, then the first rows of the profile's kind.
    Card* m_basic;
    cutefish::ui::TextField* m_name;
    PluginForm* m_plugin = nullptr;
    WireGuardEditor* m_wireGuard = nullptr;
    Card* m_switches;
    SwitchRow* m_shared;
    SwitchRow* m_ownNetworkOnly;
};

} // namespace settings::network
