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
    id: control
    headerTitle: qsTr("Bluetooth")

    property bool bluetoothDisConnected: Bluez.Manager.bluetoothBlocked

    onBluetoothDisConnectedChanged: {
        bluetoothSwitch.checked = !bluetoothDisConnected
    }

    function setBluetoothEnabled(enabled) {
        Bluez.Manager.bluetoothBlocked = !enabled

        for (var i = 0; i < Bluez.Manager.adapters.length; ++i) {
            var adapter = Bluez.Manager.adapters[i]
            adapter.powered = enabled
        }
    }

    Bluez.DevicesProxyModel {
        id: devicesProxyModel
        sourceModel: devicesModel
    }

    Bluez.DevicesModel {
        id: devicesModel
    }

    BluetoothManager {
        id: bluetoothMgr

        onShowPairDialog: {
            _pairDialog.title = name
            _pairDialog.visible = true
        }
    }

    PairDialog {
        id: _pairDialog
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
                    id: _listView
                    visible: count > 0
                    interactive: false
                    spacing: FishUI.Units.largeSpacing

                    Layout.fillWidth: true

                    Layout.preferredHeight: {
                        var totalHeight = 0
                        for (var i = 0; i < _listView.visibleChildren.length; ++i) {
                            totalHeight += _listView.visibleChildren[i].height
                        }
                        return totalHeight
                    }

                    model: devicesProxyModel //Bluez.Manager.bluetoothOperational ? devicesModel : []

                    section.property: "Section"
                    section.delegate: Label {
                        color: FishUI.Theme.disabledTextColor
                        topPadding: FishUI.Units.largeSpacing
                        bottomPadding: FishUI.Units.largeSpacing
                        text: section == "Connected" ? qsTr("Connected devices")
                                                     : qsTr("Available devices")
                    }

                    delegate: Item {
                        width: ListView.view.width
                        height: _itemLayout.implicitHeight + FishUI.Units.largeSpacing * 1.5

                        Rectangle {
                            anchors.fill: parent
                            radius: FishUI.Theme.smallRadius
                            color: FishUI.Theme.textColor
                            opacity: mouseArea.pressed ? 0.15 :  mouseArea.containsMouse ? 0.1 : 0.0
                        }

                        MouseArea {
                            id: mouseArea
                            anchors.fill: parent
                            hoverEnabled: true
                            acceptedButtons: Qt.LeftButton
                            onClicked: {
                                if (model.Connected && model.Paired){
                                    return
                                }

                                if (model.Paired) {
                                    bluetoothMgr.connectToDevice(model.Address)
                                } else {
                                    bluetoothMgr.requestParingConnection(model.Address)
                                }
                            }
                        }

                        ColumnLayout {
                            id: _itemLayout
                            anchors.fill: parent

                            Label {
                                text: model.DeviceFullName
                            }
                        }
                    }
                }
            }

            Item {
                height: FishUI.Units.largeSpacing * 2
            }
        }
    }
}
