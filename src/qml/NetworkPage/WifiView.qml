import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

Item {
    implicitHeight: _contentLayout.implicitHeight + FishUI.Theme.mediumRadius * 2

    ColumnLayout {
        id: _contentLayout
        anchors.fill: parent
        spacing: FishUI.Units.smallSpacing

        RowLayout {
            Label {
                id: wlanLabel
                text: qsTr("WLAN")
            }

            FishUI.BusyIndicator {
                id: wlanBusyIndicator
                width: wirelessSwitch.height
                height: width
                visible: enabledConnections.wirelessEnabled && wirelessView.count === 0
                running: wlanBusyIndicator.visible
            }

            Item {
                Layout.fillWidth: true
            }

            Switch {
                id: wirelessSwitch
                height: wlanLabel.implicitHeight
                leftPadding: 0
                rightPadding: 0

                checked: enabledConnections.wirelessEnabled
                onCheckedChanged: {
                    if (checked) {
                        if (!enabledConnections.wirelessEnabled) {
                            handler.enableWireless(checked)
                            handler.requestScan()
                        }
                    } else {
                        if (enabledConnections.wirelessEnabled) {
                            handler.enableWireless(checked)
                        }
                    }
                }
            }
        }

        ListView {
            id: wirelessView
            Layout.fillWidth: true
            Layout.preferredHeight: itemHeight * count + ((count - 1) * FishUI.Units.smallSpacing)
            clip: true
            model: appletProxyModel
            spacing: FishUI.Units.smallSpacing
            interactive: false

            visible: enabledConnections.wirelessEnabled

            property var itemHeight: 45

            delegate: WifiItem {
                width: ListView.view.width
                height: ListView.view.itemHeight
            }
        }
    }
}
