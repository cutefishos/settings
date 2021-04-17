import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Particles 2.12
import FishUI 1.0 as FishUI

Item {
    id: control
    height: 150

    property int value: 0
    property int radius: height * 0.2
    property bool enableAnimation: false

    Rectangle {
        id: bgRect
        anchors.fill: parent
        color: Qt.rgba(FishUI.Theme.highlightColor.r,
                       FishUI.Theme.highlightColor.g,
                       FishUI.Theme.highlightColor.b, 0.5)
        radius: control.radius

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Item {
                width: bgRect.width
                height: bgRect.height

                Rectangle {
                    anchors.fill: parent
                    radius: control.radius + 1
                }
            }
        }

        Rectangle {
            id: valueRect
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: control.width * (control.value / 100)
            color: FishUI.Theme.highlightColor
            opacity: 1

            gradient: Gradient {
                GradientStop { position: 0.0; color: Qt.rgba(FishUI.Theme.highlightColor.r,
                                                             FishUI.Theme.highlightColor.g,
                                                             FishUI.Theme.highlightColor.b, 1) }
                GradientStop { position: 1.0; color: Qt.rgba(FishUI.Theme.highlightColor.r,
                                                             FishUI.Theme.highlightColor.g,
                                                             FishUI.Theme.highlightColor.b, 0.6) }
            }

            Behavior on width {
                SmoothedAnimation {
                    velocity: 1000
                    easing.type: Easing.InOutCubic
                }
            }

            ParticleSystem {
                anchors.fill: parent

                Emitter {
                    id: emitter
                    anchors.fill: parent
                    emitRate: 7
                    lifeSpan: 2000
                    lifeSpanVariation: 500
                    size: 16
                    endSize: 32
                    enabled: control.enableAnimation

                    velocity: AngleDirection {
                        angle: 0
                        magnitude: 175
                        magnitudeVariation: 50
                    }
                }

                ItemParticle {
                    delegate: Rectangle {
                        id: particleRect
                        width: Math.ceil(Math.random() * (10 - 4)) + 4
                        height: width
                        radius: width
                        color: Qt.rgba(255, 255, 255, 0.3)
                    }
                }
            }
        }
    }
}
