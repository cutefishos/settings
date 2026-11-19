// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "cutefish/networkmanager/handler.h"
#include "cutefish/networkmanager/vpn_schema.h"
#include "cutefish/networkmanager/vpn_utils.h"

namespace settings::network {

// What a VPN profile's form holds as it is edited: its name, the values of
// its kind's fields, and who may use it. load() takes Handler::vpnForm(),
// collect() gives Handler::saveVpn() its form.
class VpnFormModel
{
public:
    // A way to keep a password, as its storage choice lists it.
    struct StorageChoice
    {
        std::string label;
        uint32_t flags = 0;
    };

    // The form's values, without secrets.
    void load(const cutefish::networkmanager::VpnForm& form);
    void setSecrets(const cutefish::networkmanager::VpnSecrets& secrets);
    cutefish::networkmanager::VpnValues collect() const;
    bool complete() const;

    const cutefish::networkmanager::VpnForm& form() const { return m_form; }
    bool installed() const { return m_form.installed; }
    // Which form the kind fills in; neither while its plugin is missing.
    bool isPlugin() const { return m_form.installed && m_form.kind != "wireguard"; }
    bool isWireGuard() const { return m_form.installed && m_form.kind == "wireguard"; }

    const std::string& name() const { return m_name; }
    void setName(const std::string& name) { m_name = name; }
    bool shared() const { return m_shared; }
    void setShared(bool shared) { m_shared = shared; }
    bool ownNetworkOnly() const { return m_ownNetworkOnly; }
    void setOwnNetworkOnly(bool ownNetworkOnly) { m_ownNetworkOnly = ownNetworkOnly; }

    // A plugin VPN's fields, by key: text as typed, "true" or "false" for Bools.
    const std::vector<cutefish::networkmanager::vpn::schema::FormField>& fields() const { return m_form.fields; }
    std::string value(const std::string& key) const;
    void setValue(const std::string& key, const std::string& value);
    // A password field's text.
    std::string secret(const std::string& key) const;
    void setSecret(const std::string& key, const std::string& secret);
    // Whether the field shows, given the values of the fields it depends on.
    bool shown(const cutefish::networkmanager::vpn::schema::Field& field) const;
    // The Choice field's choice its value is; the first when it is none.
    int choiceIndex(const cutefish::networkmanager::vpn::schema::FormField& field) const;
    // Where the password may be kept, which depends on shared(), and which
    // of those it is kept in.
    std::vector<StorageChoice> storageChoices(const cutefish::networkmanager::vpn::schema::FormField& field) const;
    int storageIndex(const cutefish::networkmanager::vpn::schema::FormField& field) const;
    void setStorageIndex(const cutefish::networkmanager::vpn::schema::FormField& field, int index);
    // The password is kept, so its field asks for it.
    bool saved(const std::string& key) const;

    // WireGuard's interface and peers, the text as typed; one peer at least
    // once loaded.
    const cutefish::networkmanager::vpn::WireGuardForm& wireGuard() const { return m_wireGuard; }
    cutefish::networkmanager::vpn::WireGuardForm& wireGuard() { return m_wireGuard; }
    // A key typed over the stored one, if any; clearing it drops the stored one.
    void setPresharedKey(size_t peer, const std::string& key);
    void addPeer();
    void removePeer(size_t peer);

private:
    cutefish::networkmanager::VpnForm m_form;
    std::string m_name;
    bool m_shared = false;
    bool m_ownNetworkOnly = false;

    cutefish::networkmanager::vpn::schema::Values m_values;
    std::map<std::string, uint32_t> m_storage;
    std::map<std::string, std::string> m_secrets;

    cutefish::networkmanager::vpn::WireGuardForm m_wireGuard;
    // The stored private key, which is kept while none is typed.
    bool m_hasPrivateKey = false;
};

} // namespace settings::network
