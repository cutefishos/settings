/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.4
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.3
import Cutefish.Settings 1.0
import FishUI 1.0 as FishUI
import Cutefish.Bluez 1.0 as Bluez
import "../"

ItemPage {
    headerTitle: qsTr("Bluetooth")

    property bool bluetoothDisConnected: Bluez.Manager.bluetoothBlocked

    onBluetoothDisConnectedChanged: {
        bluetoothSwitch.checked = !bluetoothDisConnected
    }

    function setBluetoothEnabled(enabled) {
        Bluez.Manager.bluetoothBlocked = !enabled

        for (var i = 0; i < Bluez.Manager.adapters.length; ++i) {
            var adapter = Bluez.Manager.adapters[i]
            console.log(adapter + ", " + enabled)
            adapter.powered = enabled
        }
    }

    Bluez.DevicesProxyModel {
        id: devicesModel
        sourceModel: Bluez.DevicesModel { }
    }

    BluetoothManager {
        id: bluetoothMgr
    }

//    Label {
//        id: noBluetoothMessage
//        anchors.centerIn: parent
//        visible: BluezQt.Manager.rfkill.state === BluezQt.Rfkill.Unknown
//        text: qsTr("No Bluetooth adapters found")
//    }

//    Label {
//        anchors.centerIn: parent
//        text: qsTr("Bluetooth is disabled")
//        visible: Bluez.Manager.operational && !Bluez.Manager.bluetoothOperational // && !noBluetoothMessage.visible
//    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            anchors.bottomMargin: FishUI.Units.largeSpacing

            RoundedItem {
                id: mainItem
                spacing: FishUI.Units.largeSpacing

                RowLayout {
                    Label {
                        text: qsTr("Bluetooth")
                        color: FishUI.Theme.disabledTextColor
                    }

                    Item {
                        Layout.fillWidth: true
                    }

                    Switch {
                        id: bluetoothSwitch
                        Layout.fillHeight: true
                        rightPadding: 0
                        checked: !Bluez.Manager.bluetoothBlocked
                        onCheckedChanged: setBluetoothEnabled(checked)
                    }
                }

                ListView {
                    visible: count > 0

                    property var itemHeight: 50

                    onCountChanged: {
                        console.log(count)
                    }

                    Layout.fillWidth: true
                    Layout.preferredHeight: itemHeight * count + ((count - 1) * spacing)

                    model: devicesModel //Bluez.Manager.bluetoothOperational ? devicesModel : []

                    delegate: Item {
                        width: ListView.view.itemHeight
                        height: 50

                        ColumnLayout {
                            anchors.fill: parent

                            Label {
                                text: model.DeviceFullName
                            }
                        }
                    }
                }
            }
        }
    }
}
