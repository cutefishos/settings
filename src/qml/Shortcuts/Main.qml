import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0
import "../"

ItemPage {
    id: page
    headerTitle: qsTr("Keyboard Shortcuts")

    ShortcutsModel {
        id: shortcuts
    }

    Scrollable {
        anchors.fill: parent
        contentHeight: layout.implicitHeight

        ColumnLayout {
            id: layout
            anchors.fill: parent
            spacing: Metrics.sectionSpacing

            RoundedItem {
                visible: !shortcuts.available

                FishUI.Label {
                    Layout.fillWidth: true
                    text: qsTr("Keyboard shortcuts are managed by the Cutefish compositor, which is not running.")
                    color: FishUI.Theme.disabledTextColor
                    wrapMode: Text.WordWrap
                }
            }

            FishUI.Label {
                visible: shortcuts.available
                Layout.fillWidth: true
                Layout.leftMargin: Metrics.cardHorizontalPadding
                Layout.rightMargin: Metrics.cardHorizontalPadding
                text: shortcuts.commandKeys
                      ? qsTr("Click a system shortcut to change it. Shortcuts with both Control and Command are the safest choice: applications never get them.")
                      : qsTr("Click a system shortcut to change it. Shortcuts with Meta are the safest choice: applications never use them.")
                color: FishUI.Theme.disabledTextColor
                wrapMode: Text.WordWrap
            }

            Repeater {
                model: shortcuts.categories

                ColumnLayout {
                    id: section
                    required property string modelData
                    Layout.fillWidth: true
                    visible: shortcuts.available
                    spacing: Metrics.sectionLabelSpacing

                    SectionLabel {
                        text: section.modelData
                    }

                    ListCard {
                        Repeater {
                            model: shortcuts

                            ShortcutRow {
                                visible: category === section.modelData
                                onClicked: page.edit(name)
                            }
                        }
                    }
                }
            }

            StandardButton {
                visible: shortcuts.available
                Layout.fillWidth: true
                text: qsTr("Restore all defaults")
                enabled: shortcuts.hasCustomAccelerators
                onClicked: resetAllDialog.createObject(page)
            }
        }
    }

    function edit(actionName) {
        editLoader.actionName = actionName
        editLoader.active = false
        editLoader.active = true
    }

    Loader {
        id: editLoader
        property string actionName
        active: false
        sourceComponent: ShortcutEditDialog {
            shortcutsModel: shortcuts
            actionName: editLoader.actionName
            onClosing: editLoader.active = false
        }
        onLoaded: item.show()
    }

    Component {
        id: resetAllDialog

        FishUI.ConfirmDialog {
            title: qsTr("Restore all defaults")
            text: qsTr("Every shortcut goes back to its default. Your changes will be lost.")
            cancelText: qsTr("Cancel")
            confirmText: qsTr("Restore")
            destructive: true
            onAccepted: shortcuts.restoreAllDefaults()
            onClosing: destroy()
        }
    }
}
