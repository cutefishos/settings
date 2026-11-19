// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <map>
#include <string>

#include "cutefish/networkmanager/handler.h"
#include "cutefish/networkmanager/types.h"
#include "cutefish/networkmanager/vpn_schema.h"
#include "pages/network/vpn/vpnformmodel.h"

using namespace settings::network;
namespace nm = cutefish::networkmanager;
namespace schema = cutefish::networkmanager::vpn::schema;

static nm::VpnForm openVpnForm(const std::map<std::string, std::string>& data)
{
    nm::VpnForm form;
    form.kind = "openvpn";
    form.label = "OpenVPN";
    form.installed = true;
    form.name = "Office";
    form.fields = schema::formFields("openvpn", data);
    return form;
}

TEST(VpnForm, PluginFormNeedsItsServer)
{
    VpnFormModel model;
    model.load(openVpnForm({}));
    EXPECT_FALSE(model.complete());

    model.load(openVpnForm({{"remote", "vpn.example.com"}, {"connection-type", "password"}, {"username", "alice"}}));
    EXPECT_TRUE(model.complete());
    const nm::VpnValues values = model.collect();
    EXPECT_EQ(values.kind, "openvpn");
    EXPECT_EQ(values.name, "Office");
    EXPECT_EQ(values.values.at("remote"), "vpn.example.com");
    EXPECT_EQ(values.values.at("username"), "alice");
    // Bools go back as text, false while off.
    EXPECT_EQ(values.values.at("proto-tcp"), "false");
    // A password with no flags is this user's, kept in the keyring.
    EXPECT_EQ(values.storage.at("password"), uint32_t(nm::SecretAgentOwned));
}

TEST(VpnForm, SecretsFillThePasswords)
{
    VpnFormModel model;
    model.load(openVpnForm({{"remote", "vpn.example.com"}, {"connection-type", "password"}}));
    nm::VpnSecrets secrets;
    secrets.values = {{"password", "s3cret"}, {"unknown", "dropped"}};
    model.setSecrets(secrets);
    const nm::VpnValues values = model.collect();
    EXPECT_EQ(values.secrets.at("password"), "s3cret");
    EXPECT_FALSE(values.secrets.count("unknown"));
}

TEST(VpnForm, MissingPluginShowsNoForm)
{
    nm::VpnForm form;
    form.kind = "l2tp";
    form.installed = false;
    form.package = "network-manager-l2tp";
    VpnFormModel model;
    model.load(form);
    EXPECT_FALSE(model.installed());
    EXPECT_FALSE(model.complete());
}

TEST(VpnForm, WireGuardKeepsItsPeers)
{
    nm::VpnForm form;
    form.kind = "wireguard";
    form.installed = true;
    form.wireguard.addresses = "10.0.0.2/24";
    form.wireguard.peers = {{.publicKey = "xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg=", .keepalive = "25"}};
    VpnFormModel model;
    model.load(form);
    // The private key is stored, not shown: without it the form isn't complete.
    EXPECT_FALSE(model.complete());
    nm::VpnSecrets secrets;
    secrets.privateKey = "yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=";
    model.setSecrets(secrets);
    EXPECT_TRUE(model.complete());

    const nm::VpnValues values = model.collect();
    EXPECT_EQ(values.wireguard.privateKey, secrets.privateKey);
    EXPECT_EQ(values.wireguard.addresses, "10.0.0.2/24");
    ASSERT_EQ(values.wireguard.peers.size(), 1u);
    EXPECT_EQ(values.wireguard.peers[0].keepalive, "25");
    EXPECT_TRUE(values.wireguard.peerRoutes);
}

TEST(VpnForm, RemovingAWireGuardPeerKeepsTheOthers)
{
    nm::VpnForm form;
    form.kind = "wireguard";
    form.installed = true;
    form.wireguard.peers = {{.publicKey = "first"}, {.publicKey = "second"}};
    VpnFormModel model;
    model.load(form);

    model.removePeer(0);
    const nm::VpnValues values = model.collect();
    ASSERT_EQ(values.wireguard.peers.size(), 1u);
    EXPECT_EQ(values.wireguard.peers[0].publicKey, "second");
}
