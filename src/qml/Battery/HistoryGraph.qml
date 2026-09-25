import QtQuick
import QtQuick.Shapes
import FishUI 1.0 as FishUI
import Cutefish.Settings 1.0

// Plots history samples over the `duration` seconds up to `end` as a smooth
// curve. Labels are native text and grid lines sit on the device pixel grid,
// so both stay sharp at a fractional scale; the curve is a single Shape.
Item {
    id: control

    property var points: [] // QPointF: x in Unix seconds, y the value
    property real end: 0 // Unix seconds at the right edge
    property real duration: 86400
    property real yMax: 100
    property real yStep: 20
    property string yUnits: ""

    readonly property real start: end - duration

    // The time ticks as one value, computed from `end` and `duration` alone: a
    // count chained through other bindings can be evaluated against a stale
    // start when `end` first arrives, and ask the Repeater for 100000 ticks.
    readonly property var ticks: {
        const steps = [600, 1800, 3600, 7200, 14400, 21600, 43200, 86400]
        let step = Math.ceil(duration / 6 / 86400) * 86400

        // The smallest whole step that leaves six or fewer ticks.
        for (let i = 0; i < steps.length; ++i) {
            if (duration / steps[i] <= 6) {
                step = steps[i]
                break
            }
        }

        // On a whole local multiple of the step, e.g. midnight for every 4 hours.
        const from = end - duration
        const offset = new Date(from * 1000).getTimezoneOffset() * 60
        const first = Math.ceil((from - offset) / step) * step + offset

        return {
            step: step,
            first: first,
            count: end < first ? 0 : Math.floor((end - first) / step) + 1
        }
    }

    readonly property color lineColor: FishUI.Theme.highlightColor
    readonly property color gridColor: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.1)
                                                             : Qt.rgba(0, 0, 0, 0.08)
    readonly property font labelFont: Qt.font({
        family: FishUI.Theme.fontFamily,
        pointSize: Math.max(FishUI.Theme.fontSize - 1, 7)
    })

    // Both SVG paths, built in one pass whenever the samples or the size change.
    // The curve is a monotone cubic (Steffen's method): it bends smoothly but
    // never overshoots a sample, so a full battery cannot read above 100%.
    readonly property var paths: {
        const width = plot.width
        const height = plot.height
        const xs = []
        const ys = []

        for (let i = 0; i < points.length; ++i) {
            const p = points[i]

            if (p.x < start)
                continue

            xs.push((p.x - start) / duration * width)
            ys.push(height - Math.min(p.y, yMax) / yMax * height)
        }

        const n = xs.length

        if (n < 2)
            return { line: "", area: "" }

        const tangents = new Array(n)

        for (let i = 1; i < n - 1; ++i) {
            const h0 = xs[i] - xs[i - 1]
            const h1 = xs[i + 1] - xs[i]
            const s0 = (ys[i] - ys[i - 1]) / h0
            const s1 = (ys[i + 1] - ys[i]) / h1
            const p = (s0 * h1 + s1 * h0) / (h0 + h1)

            tangents[i] = (Math.sign(s0) + Math.sign(s1))
                          * Math.min(Math.abs(s0), Math.abs(s1), 0.5 * Math.abs(p)) || 0
        }

        const firstSlope = (ys[1] - ys[0]) / (xs[1] - xs[0])
        const lastSlope = (ys[n - 1] - ys[n - 2]) / (xs[n - 1] - xs[n - 2])
        tangents[0] = n > 2 ? (3 * firstSlope - tangents[1]) / 2 : firstSlope
        tangents[n - 1] = n > 2 ? (3 * lastSlope - tangents[n - 2]) / 2 : lastSlope

        let line = "M" + xs[0] + "," + ys[0]

        for (let i = 0; i < n - 1; ++i) {
            const d = (xs[i + 1] - xs[i]) / 3

            line += "C" + (xs[i] + d) + "," + (ys[i] + d * tangents[i])
                    + " " + (xs[i + 1] - d) + "," + (ys[i + 1] - d * tangents[i + 1])
                    + " " + xs[i + 1] + "," + ys[i + 1]
        }

        return {
            line: line,
            area: line + "L" + xs[n - 1] + "," + height + "L" + xs[0] + "," + height + "Z"
        }
    }

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
        // Two label lines below: the time, and the date where a day starts.
        height: control.height - y - Math.ceil(labelMetrics.height) * 2 - Metrics.s

        Repeater {
            model: Math.floor(control.yMax / control.yStep) + 1

            FishUI.RoundedRectangle {
                id: gridLine

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
                    text: gridLine.value + control.yUnits
                    font: control.labelFont
                    color: FishUI.Theme.disabledTextColor
                    renderType: Text.NativeRendering
                }
            }
        }

        // Indexed rather than a list of times, so the ticks are kept and only
        // moved as the right edge follows the clock.
        Repeater {
            model: control.ticks.count

            FishUI.RoundedRectangle {
                id: tick

                readonly property real time: control.ticks.first + index * control.ticks.step

                x: control.xFor(time) - width / 2
                width: 1 / FishUI.Dpi.ratio
                height: plot.height
                continuousCorners: false
                color: control.gridColor

                FishUI.Label {
                    // Centred under the tick, but kept inside the graph at either end.
                    x: Math.max(-tick.x, Math.min(control.width - plot.x - tick.x - width, -width / 2))
                    anchors.top: parent.bottom
                    anchors.topMargin: Metrics.s
                    horizontalAlignment: Text.AlignHCenter
                    lineHeight: 1.1
                    text: {
                        const date = new Date(tick.time * 1000)
                        const time = date.toLocaleTimeString(Qt.locale(), Locale.ShortFormat)

                        // A day starts here; name it under the time.
                        if (date.getHours() === 0 && date.getMinutes() === 0)
                            return time + "\n" + date.toLocaleDateString(Qt.locale(), Locale.ShortFormat)

                        return time
                    }
                    font: control.labelFont
                    color: FishUI.Theme.disabledTextColor
                    renderType: Text.NativeRendering
                }
            }
        }

        Shape {
            anchors.fill: parent
            preferredRendererType: Shape.CurveRenderer

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

                PathSvg {
                    path: control.paths.area
                }
            }

            ShapePath {
                strokeColor: control.lineColor
                strokeWidth: 2
                fillColor: "transparent"
                joinStyle: ShapePath.RoundJoin
                capStyle: ShapePath.RoundCap

                PathSvg {
                    path: control.paths.line
                }
            }
        }
    }
}
