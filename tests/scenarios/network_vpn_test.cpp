// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <map>
#include <memory>
#include <string>

#include "cutefish/networkmanager/handler.h"
#include "cutefish/networkmanager/vpn_schema.h"
#include "pages/network/vpn/vpneditor.h"
#include "scenario.h"
#include "ui/window.h"

using namespace settings::network;
namespace nm = cutefish::networkmanager;
namespace schema = cutefish::networkmanager::vpn::schema;

// The editor alone in a headless window, loaded with the form.
struct EditorWindow
{
    explicit EditorWindow(const nm::VpnForm& form)
    {
        auto content = std::make_unique<VpnEditor>();
        editor = content.get();
        window.setContentView(std::move(content));
        window.resize({480, 1200});
        editor->load(form);
        window.show();
        driver.setKeyboardFocus(true);
        driver.layout();
    }

    cutefish::ui::Window window;
    VpnEditor* editor = nullptr;
    cutefish::ui::TestDriver driver{window, scenarioFailure};
};

TEST(VpnEditor, TypingFillsThePluginForm)
{
    nm::VpnForm form;
    form.kind = "openvpn";
    form.label = "OpenVPN";
    form.installed = true;
    form.name = "Office";
    form.fields = schema::formFields("openvpn", {{"connection-type", "password"}});
    EditorWindow editor(form);
    EXPECT_FALSE(editor.editor->complete());

    editor.driver.click("vpn:field:remote");
    editor.driver.typeText("vpn.example.com");
    EXPECT_TRUE(editor.editor->complete());
    replaceText(editor.driver, "vpn:name", "Home");
    editor.driver.click("vpn:field:password");
    editor.driver.typeText("s3cret");

    const nm::VpnValues values = editor.editor->collect();
    EXPECT_EQ(values.name, "Home");
    EXPECT_EQ(values.values.at("remote"), "vpn.example.com");
    EXPECT_EQ(values.secrets.at("password"), "s3cret");
}

TEST(VpnEditor, TypingFillsTheWireGuardForm)
{
    nm::VpnForm form;
    form.kind = "wireguard";
    form.installed = true;
    EditorWindow editor(form);
    // A tunnel starts with one peer to fill in.
    EXPECT_FALSE(editor.editor->complete());

    editor.driver.click("vpn:privateKey");
    editor.driver.typeText("yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=");
    editor.driver.click("vpn:address");
    editor.driver.typeText("10.0.0.2/24");
    EXPECT_FALSE(editor.editor->complete());
    editor.driver.click("vpn:publicKey:0");
    editor.driver.typeText("xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg=");
    EXPECT_TRUE(editor.editor->complete());

    const nm::VpnValues values = editor.editor->collect();
    EXPECT_EQ(values.wireguard.privateKey, "yAnz5TF+lXXJte14tji3zlMNq+hd2rYUIgJBgB3fBmk=");
    EXPECT_EQ(values.wireguard.addresses, "10.0.0.2/24");
    ASSERT_EQ(values.wireguard.peers.size(), 1u);
    EXPECT_EQ(values.wireguard.peers[0].publicKey, "xTIBA5rboUvnH4htodjb6e697QjLERt1NAB4mZqp8Dg=");
}

TEST(VpnEditor, RemovingAWireGuardPeerKeepsTheOthers)
{
    nm::VpnForm form;
    form.kind = "wireguard";
    form.installed = true;
    form.wireguard.peers = {{.publicKey = "first"}, {.publicKey = "second"}};
    EditorWindow editor(form);

    // The button goes with the peer's card it sits in.
    ASSERT_TRUE(editor.driver.click("vpn:removePeer:0"));

    const nm::VpnValues values = editor.editor->collect();
    ASSERT_EQ(values.wireguard.peers.size(), 1u);
    EXPECT_EQ(values.wireguard.peers[0].publicKey, "second");
}
