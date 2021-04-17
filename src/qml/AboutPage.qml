import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0

ItemPage {
    headerTitle: qsTr("About")

    About {
        id: about
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent

            Image {
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                width: 128
                height: width
                sourceSize: Qt.size(width, height)
                source: "qrc:/images/logo.svg"
            }

            Label {
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                text: "<b>CutefishOS</b>"
                font.pointSize: 24
                color: "#3385FF"
                leftPadding: FishUI.Units.largeSpacing * 2
                rightPadding: FishUI.Units.largeSpacing * 2
            }

            Item {
                height: FishUI.Units.largeSpacing
            }

//            Label {
//                text: qsTr("Built on ") + about.prettyProductName
//                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
//                bottomPadding: FishUI.Units.largeSpacing * 2
//                color: FishUI.Theme.disabledTextColor
//            }

            RoundedItem {
                StandardItem {
                    key: qsTr("System Version")
                    value: "0.2"
                }

                StandardItem {
                    key: qsTr("System Type")
                    value: about.architecture
                }

                StandardItem {
                    key: qsTr("Kernel Version")
                    value: about.kernelVersion
                }

                StandardItem {
                    key: qsTr("Processor")
                    value: about.cpuInfo
                }

                StandardItem {
                    key: qsTr("RAM")
                    value: about.memorySize
                }

                StandardItem {
                    key: qsTr("Internal Storage")
                    value: about.internalStorage
                }
            }
        }
    }
}
