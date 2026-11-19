// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/network/vpn/vpnformmodel.h"

#include <algorithm>

#include "base/i18n.h"
#include "cutefish/networkmanager/types.h"

using cutefish::i18n::tr;
namespace nm = cutefish::networkmanager;
namespace schema = cutefish::networkmanager::vpn::schema;

namespace settings::network {

static bool blank(const std::string& text)
{
    return text.find_first_not_of(" \t\n") == std::string::npos;
}

void VpnFormModel::load(const nm::VpnForm& form)
{
    m_form = form;
    m_name = form.name;
    m_shared = form.shared;
    m_ownNetworkOnly = form.ownNetworkOnly;

    m_values.clear();
    m_storage.clear();
    m_secrets.clear();
    if (isPlugin()) {
        for (const schema::FormField& field : m_form.fields) {
            m_values[field.key] = field.type == schema::Type::Bool ? (field.checked ? "true" : "false") : field.value;
            if (field.type == schema::Type::Password)
                m_storage[field.key] = field.storage;
        }
    }

    m_wireGuard = {};
    m_hasPrivateKey = false;
    if (isWireGuard()) {
        m_wireGuard = form.wireguard;
        // The profile's name is the form's, and the private key is stored,
        // not shown.
        m_wireGuard.name.clear();
        m_wireGuard.privateKey.clear();
        if (m_wireGuard.peers.empty())
            addPeer();
    }
}

void VpnFormModel::setSecrets(const nm::VpnSecrets& secrets)
{
    if (isPlugin()) {
        m_secrets.clear();
        for (const schema::FormField& field : m_form.fields) {
            const auto found = secrets.values.find(field.key);
            if (field.type == schema::Type::Password && found != secrets.values.end())
                m_secrets[field.key] = found->second;
        }
    }
    if (isWireGuard()) {
        if (!secrets.privateKey.empty()) {
            m_wireGuard.privateKey = secrets.privateKey;
            m_hasPrivateKey = true;
        }
        for (nm::vpn::WireGuardPeer& peer : m_wireGuard.peers) {
            const auto key = secrets.presharedKeys.find(peer.publicKey);
            if (key != secrets.presharedKeys.end())
                peer.presharedKey = key->second;
        }
    }
}

nm::VpnValues VpnFormModel::collect() const
{
    nm::VpnValues values;
    values.kind = m_form.kind;
    values.name = m_name;
    values.shared = m_shared;
    values.ownNetworkOnly = m_ownNetworkOnly;
    if (isPlugin()) {
        values.values = m_values;
        values.storage = m_storage;
        values.secrets = m_secrets;
    }
    if (isWireGuard())
        values.wireguard = m_wireGuard;
    return values;
}

bool VpnFormModel::complete() const
{
    if (isPlugin()) {
        for (const schema::FormField& field : m_form.fields) {
            if (field.required && shown(field) && blank(value(field.key)))
                return false;
        }
        return true;
    }
    if (isWireGuard()) {
        if (blank(m_wireGuard.addresses))
            return false;
        if (!m_hasPrivateKey && blank(m_wireGuard.privateKey))
            return false;
        if (m_wireGuard.peers.empty())
            return false;
        return std::none_of(m_wireGuard.peers.begin(), m_wireGuard.peers.end(),
                            [](const nm::vpn::WireGuardPeer& peer) { return blank(peer.publicKey); });
    }
    return false;
}

std::string VpnFormModel::value(const std::string& key) const
{
    const auto found = m_values.find(key);
    return found == m_values.end() ? std::string() : found->second;
}

void VpnFormModel::setValue(const std::string& key, const std::string& value)
{
    m_values[key] = value;
}

std::string VpnFormModel::secret(const std::string& key) const
{
    const auto found = m_secrets.find(key);
    return found == m_secrets.end() ? std::string() : found->second;
}

void VpnFormModel::setSecret(const std::string& key, const std::string& secret)
{
    m_secrets[key] = secret;
}

bool VpnFormModel::shown(const schema::Field& field) const
{
    std::vector<schema::Field> plain(m_form.fields.begin(), m_form.fields.end());
    return schema::shown(field, plain, m_values);
}

int VpnFormModel::choiceIndex(const schema::FormField& field) const
{
    const std::string text = value(field.key);
    int index = 0;
    for (size_t i = 0; i < field.choices.size(); ++i) {
        if (field.choices[i].value == text)
            index = int(i);
    }
    return index;
}

std::vector<VpnFormModel::StorageChoice> VpnFormModel::storageChoices(const schema::FormField& field) const
{
    std::vector<StorageChoice> choices{
        {m_shared ? tr("Saved for all") : tr("Saved"), m_shared ? nm::SecretSystemOwned : nm::SecretAgentOwned},
        {tr("Ask each time"), nm::SecretNotSaved},
    };
    if (field.allowNotRequired)
        choices.push_back({tr("Not needed"), nm::SecretNotRequired});
    return choices;
}

int VpnFormModel::storageIndex(const schema::FormField& field) const
{
    const uint32_t saveFlags = m_shared ? nm::SecretSystemOwned : nm::SecretAgentOwned;
    const auto found = m_storage.find(field.key);
    const uint32_t stored = found == m_storage.end() ? nm::SecretAgentOwned : found->second;
    const uint32_t wanted = stored == 0 || stored == 1 ? saveFlags : stored;
    const std::vector<StorageChoice> choices = storageChoices(field);
    const auto index = std::find_if(choices.begin(), choices.end(),
                                    [wanted](const StorageChoice& choice) { return choice.flags == wanted; });
    return index == choices.end() ? 0 : int(index - choices.begin());
}

void VpnFormModel::setStorageIndex(const schema::FormField& field, int index)
{
    const std::vector<StorageChoice> choices = storageChoices(field);
    if (index >= 0 && size_t(index) < choices.size())
        m_storage[field.key] = choices[size_t(index)].flags;
}

bool VpnFormModel::saved(const std::string& key) const
{
    const auto found = m_storage.find(key);
    const uint32_t storage = found == m_storage.end() ? nm::SecretAgentOwned : found->second;
    return storage != nm::SecretNotSaved && storage != nm::SecretNotRequired;
}

void VpnFormModel::setPresharedKey(size_t peer, const std::string& key)
{
    if (peer >= m_wireGuard.peers.size())
        return;
    m_wireGuard.peers[peer].presharedKey = key;
    m_wireGuard.peers[peer].hasPresharedKey = !key.empty();
}

void VpnFormModel::addPeer()
{
    m_wireGuard.peers.push_back({});
}

void VpnFormModel::removePeer(size_t peer)
{
    if (peer < m_wireGuard.peers.size())
        m_wireGuard.peers.erase(m_wireGuard.peers.begin() + ptrdiff_t(peer));
}

} // namespace settings::network
