// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

import QtQuick
import QtTest
import Cutefish.Network as NM
import "../../src/qml/WLAN" as WLAN

TestCase {
    id: testCase
    name: "WifiJoinDialog"
    when: windowShown

    readonly property var library: ({
        device: "/org/freedesktop/NetworkManager/Devices/2",
        name: "Library",
        security: NM.Enums.Wpa2Eap
    })
    readonly property string libraryAccessPoint: "/org/freedesktop/NetworkManager/AccessPoint/5"

    WLAN.NewNetworkDialog {
        id: dialog
    }

    SignalSpy {
        id: joinSpy
        target: dialog
        signalName: "joinNetwork"
    }

    SignalSpy {
        id: connectSpy
        target: dialog
        signalName: "connect"
    }

    function find(item, predicate) {
        if (predicate(item))
            return item
        for (var i = 0; i < item.children.length; ++i) {
            var found = find(item.children[i], predicate)
            if (found)
                return found
        }
        return null
    }

    function field(placeholder) {
        return find(dialog.contentItem, item => item.placeholderText === placeholder)
    }

    function securityBox() {
        return find(dialog.contentItem, item => item.valueRole === "value")
    }

    function init() {
        joinSpy.clear()
        connectSpy.clear()
    }

    function cleanup() {
        dialog.close()
        tryCompare(dialog, "visible", false)
    }

    function test_joinsAListedEnterpriseNetwork() {
        dialog.openNetwork(libraryAccessPoint, library)
        tryCompare(dialog, "visible", true)
        compare(dialog.title, "Join “Library”")
        compare(dialog.security, "wpa2-eap")

        // The network is known; only the sign-in is asked for, user name first.
        verify(!field("Network Name").visible)
        verify(!securityBox().visible)
        verify(field("Username").visible)
        verify(field("Password").visible)
        tryVerify(() => field("Username").activeFocus)

        verify(!dialog.canJoin)
        field("Username").text = "alice"
        verify(!dialog.canJoin)
        field("Password").text = "secret"
        verify(dialog.canJoin)

        dialog.join()
        compare(joinSpy.count, 1)
        compare(joinSpy.signalArguments[0][0], library.device)
        compare(joinSpy.signalArguments[0][1], libraryAccessPoint)
        compare(joinSpy.signalArguments[0][2], "alice")
        compare(joinSpy.signalArguments[0][3], "secret")
        compare(connectSpy.count, 0)
        tryCompare(dialog, "visible", false)
    }

    function test_wpaEnterpriseKeepsItsKind() {
        dialog.openNetwork(libraryAccessPoint, {device: library.device, name: "Old", security: NM.Enums.WpaEap})
        tryCompare(dialog, "visible", true)
        compare(dialog.security, "wpa-eap")
    }

    function test_certificateNetworkCannotBeJoined() {
        dialog.openNetwork(libraryAccessPoint,
                           {device: library.device, name: "Secure", security: NM.Enums.Wpa3SuiteB192})
        tryCompare(dialog, "visible", true)
        verify(dialog.needsCertificate)
        verify(!field("Username").visible)
        verify(!field("Password").visible)

        field("Username").text = "alice"
        field("Password").text = "secret"
        verify(!dialog.canJoin)
        dialog.join()
        compare(joinSpy.count, 0)
    }

    function test_goneNetworkOpensNothing() {
        // Handler.wirelessNetwork() is empty once the access point has gone.
        dialog.openNetwork(libraryAccessPoint, {})
        wait(50)
        verify(!dialog.visible)
    }

    function test_closingReturnsToHiddenNetworks() {
        dialog.openNetwork(libraryAccessPoint, library)
        tryCompare(dialog, "visible", true)
        field("Username").text = "alice"
        dialog.close()
        tryCompare(dialog, "visible", false)

        compare(dialog.accessPoint, "")
        compare(dialog.title, "Add Other Network")
        compare(dialog.security, "wpa2-wpa3")
        compare(field("Username").text, "")

        dialog.show()
        tryCompare(dialog, "visible", true)
        verify(field("Network Name").visible)
        verify(securityBox().visible)
        field("Network Name").text = "Garden"
        field("Password").text = "garden pass"
        dialog.join()
        compare(connectSpy.count, 1)
        compare(connectSpy.signalArguments[0][0], "Garden")
        compare(joinSpy.count, 0)
    }
}
