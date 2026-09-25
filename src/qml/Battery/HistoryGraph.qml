/*
 * Copyright (C) 2015  David Edmundson <david@davidedmundson.co.uk>
 * Copyright (C) 2021  Reion Wong <reionwong@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */


import QtQuick
import QtQuick.Shapes
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0

// A connected plot of history samples over the last `duration` seconds,
// ending at the present. Labels and grid lines are scene graph items rather
// than a Canvas, so the text is hinted and the hairlines sit on the pixel grid.
Item {
    id: control

    property var points: [] // QPointF: x in Unix seconds, y the value
    property real duration: 86400
    property real yMax: 100
    property real yStep: 20
    property string yUnits: ""

    // The right edge, in Unix seconds; taken again whenever the samples change.
    property real now: Date.now() / 1000

    readonly property real start: now - duration

    // The spacing of the time ticks: the smallest whole step leaving six or fewer.
    readonly property real tickStep: {
        const steps = [600, 1800, 3600, 7200, 14400, 21600, 43200, 86400]

        for (let i = 0; i < steps.length; ++i) {
            if (duration / steps[i] <= 6)
                return steps[i]
        }

        return Math.ceil(duration / 6 / 86400) * 86400
    }

    // Tick times on whole local hours, e.g. every 4 hours from midnight.
    readonly property var ticks: {
        const offset = new Date(now * 1000).getTimezoneOffset() * 60
        const result = []

        for (let t = Math.ceil((start - offset) / tickStep) * tickStep + offset; t <= now; t += tickStep)
            result.push(t)

        return result
    }

    readonly property color lineColor: FishUI.Theme.highlightColor
    readonly property color gridColor: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.1)
                                                             : Qt.rgba(0, 0, 0, 0.08)
    readonly property font labelFont: Qt.font({
        family: FishUI.Theme.fontFamily,
        pointSize: Math.max(FishUI.Theme.fontSize - 1, 7)
    })

    onPointsChanged: now = Date.now() / 1000

    function xFor(time) {
        return (time - start) / duration * plot.width
    }

    function yFor(value) {
        return plot.height - Math.min(value, yMax) / yMax * plot.height
    }

    TextMetrics {
        id: yLabelMetrics
        font: control.labelFont
        text: control.yMax + control.yUnits
    }

    FontMetrics {
        id: labelMetrics
        font: control.labelFont
    }

    Item {
        id: plot
        x: Math.ceil(yLabelMetrics.advanceWidth) + Metrics.s
        // Half a label above the top line, so its label is not cut off.
        y: Math.ceil(labelMetrics.height / 2)
        width: control.width - x
        height: control.height - y - Math.ceil(labelMetrics.height) * 2 - Metrics.s

        Repeater {
            model: Math.floor(control.yMax / control.yStep) + 1

            FishUI.RoundedRectangle {
                readonly property real value: index * control.yStep

                y: control.yFor(value) - height / 2
                width: plot.width
                height: 1 / FishUI.Dpi.ratio
                continuousCorners: false
                color: control.gridColor

                FishUI.Label {
                    anchors.right: parent.left
                    anchors.rightMargin: Metrics.s
                    anchors.verticalCenter: parent.verticalCenter
                    text: parent.value + control.yUnits
                    font: control.labelFont
                    color: FishUI.Theme.disabledTextColor
                    renderType: Text.NativeRendering
                }
            }
        }

        Repeater {
            model: control.ticks

            FishUI.RoundedRectangle {
                readonly property date time: new Date(modelData * 1000)
                readonly property bool midnight: time.getHours() === 0 && time.getMinutes() === 0

                x: control.xFor(modelData) - width / 2
                width: 1 / FishUI.Dpi.ratio
                height: plot.height
                continuousCorners: false
                color: control.gridColor

                FishUI.Label {
                    // Centred under the tick, but kept inside the graph at either end.
                    x: Math.max(-parent.x, Math.min(control.width - plot.x - parent.x - width, -width / 2))
                    anchors.top: parent.bottom
                    anchors.topMargin: Metrics.s
                    horizontalAlignment: Text.AlignHCenter
                    lineHeight: 1.1
                    text: {
                        const label = parent.time.toLocaleTimeString(Qt.locale(), Locale.ShortFormat)

                        // The day changes here; name it under its first tick.
                        if (parent.midnight)
                            return label + "\n" + parent.time.toLocaleDateString(Qt.locale(), Locale.ShortFormat)

                        return label
                    }
                    font: control.labelFont
                    color: FishUI.Theme.disabledTextColor
                    renderType: Text.NativeRendering
                }
            }
        }

        Shape {
            id: curve
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer

            readonly property var line: {
                const result = []

                for (let i = 0; i < control.points.length; ++i) {
                    const p = control.points[i]

                    if (p.x >= control.start)
                        result.push(Qt.point(control.xFor(p.x), control.yFor(p.y)))
                }

                return result
            }

            readonly property var area: line.length < 2 ? []
                                        : line.concat([Qt.point(line[line.length - 1].x, plot.height),
                                                       Qt.point(line[0].x, plot.height)])

            ShapePath {
                strokeColor: "transparent"
                strokeWidth: -1
                fillGradient: LinearGradient {
                    x1: 0
                    y1: 0
                    x2: 0
                    y2: plot.height
                    GradientStop { position: 0; color: Qt.rgba(control.lineColor.r, control.lineColor.g, control.lineColor.b, 0.25) }
                    GradientStop { position: 1; color: Qt.rgba(control.lineColor.r, control.lineColor.g, control.lineColor.b, 0.02) }
                }

                PathPolyline {
                    path: curve.area
                }
            }

            ShapePath {
                strokeColor: control.lineColor
                strokeWidth: 2
                fillColor: "transparent"
                joinStyle: ShapePath.RoundJoin
                capStyle: ShapePath.RoundCap

                PathPolyline {
                    path: curve.line
                }
            }
        }
    }
}
