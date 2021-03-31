import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import MeuiKit 1.0 as Meui
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
                text: "CutefishOS"
                font.pointSize: 24
                font.bold: true
                leftPadding: Meui.Units.largeSpacing * 2
                rightPadding: Meui.Units.largeSpacing * 2
            }

            Label {
                text: qsTr("Built on ") + about.prettyProductName
                Layout.alignment: Qt.AlignTop | Qt.AlignHCenter
                bottomPadding: Meui.Units.largeSpacing * 2
                color: Meui.Theme.disabledTextColor
            }

            StandardItem {
                key: qsTr("System Version")
                value: "0.1"
            }

            StandardItem {
                key: qsTr("Kernel Version")
                value: about.kernelVersion
            }

            StandardItem {
                key: qsTr("RAM")
                value: about.memorySize
            }
        }
    }
}
