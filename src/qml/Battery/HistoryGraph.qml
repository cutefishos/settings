import QtQuick
import QtQuick.Shapes
import FishUI as FishUI
import Cutefish.Settings

// Plots history samples over the `duration` seconds up to `end` as a smooth
// curve, coloured by whether the computer was plugged in. Labels are native
// text and grid lines sit on the device pixel grid, so both stay sharp at a
// fractional scale; each colour of the curve is one ShapePath.
Item {
    id: control

    property var points: [] // QPointF: x in Unix seconds, y the value
    property var pointStates: [] // UPower device state of each point
    property real end: 0 // Unix seconds at the right edge
    property real duration: 86400
    property real yMax: 100
    property real yStep: 25
    property string yUnits: ""
    property int decimals: 0

    readonly property real start: end - duration

    // The time ticks as one value, computed from `end`, `duration` and the width
    // alone: a count chained through other bindings can be evaluated against a
    // stale start when `end` first arrives, and ask the Repeater for 100000 ticks.
    readonly property var ticks: {
        const steps = [600, 1800, 3600, 7200, 14400, 21600, 43200, 86400]
        // Six steps at most, and fewer where the labels would run together.
        const fit = Math.floor(plot.width / (timeLabelMetrics.advanceWidth + Metrics.l))
        const most = Math.max(1, Math.min(6, fit))
        let step = Math.ceil(duration / most / 86400) * 86400

        // The smallest whole step that leaves that many ticks or fewer.
        for (let i = 0; i < steps.length; ++i) {
            if (duration / steps[i] <= most) {
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

    readonly property color powerColor: FishUI.Theme.greenColor
    // A green accent would hide the plugged-in runs, so blue stands in for it.
    readonly property color batteryColor: Qt.colorEqual(FishUI.Theme.highlightColor, powerColor)
                                          ? FishUI.Theme.blueColor : FishUI.Theme.highlightColor
    readonly property color surfaceColor: FishUI.Theme.secondBackgroundColor
    readonly property color gridColor: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.08)
                                                             : Qt.rgba(0, 0, 0, 0.06)
    readonly property color baselineColor: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.18)
                                                                 : Qt.rgba(0, 0, 0, 0.14)
    readonly property real areaOpacity: FishUI.Theme.darkMode ? 0.3 : 0.2
    readonly property font labelFont: Qt.font({
        family: FishUI.Theme.fontFamily,
        pointSize: Math.max(FishUI.Theme.fontSize - 1, 7)
    })

    function tint(color, alpha) {
        return Qt.rgba(color.r, color.g, color.b, alpha)
    }

    // Charging, fully charged, or held by the charge limit.
    function pluggedIn(state) {
        return state === 1 || state === 4 || state === 5
    }

    function stateText(state) {
        switch (state) {
        case 1:
            return qsTr("Charging")
        case 4:
            return qsTr("Fully charged")
        case 5:
            return qsTr("Not charging")
        case 2:
        case 3:
        case 6:
            return qsTr("On battery")
        }

        return ""
    }

    // The curve, built in one pass whenever the samples or the size change.
    // It is a monotone cubic (Steffen's method): it bends smoothly but never
    // overshoots a sample, so a full battery cannot read above 100%. UPower
    // records a sample when the value changes, with the state that changed it,
    // so the segment leading up to a sample takes that sample's state.
    readonly property var curve: {
        const width = plot.width
        const height = plot.height
        const xs = []
        const ys = []
        const sampleStates = []

        for (let i = 0; i < points.length; ++i) {
            const p = points[i]

            if (p.x < start)
                continue

            xs.push((p.x - start) / duration * width)
            ys.push(height - Math.min(p.y, yMax) / yMax * height)
            sampleStates.push(i < pointStates.length ? pointStates[i] : 0)
        }

        const n = xs.length

        if (n < 2)
            return { count: 0 }

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

        // One subpath per run of segments in the same colour.
        const lines = { battery: "", power: "" }
        const areas = { battery: "", power: "" }
        let runStart = 0
        let run = ""

        for (let i = 0; i < n - 1; ++i) {
            const d = (xs[i + 1] - xs[i]) / 3
            const power = pluggedIn(sampleStates[i + 1])

            if (i === runStart)
                run = "M" + xs[i] + "," + ys[i]

            run += "C" + (xs[i] + d) + "," + (ys[i] + d * tangents[i])
                   + " " + (xs[i + 1] - d) + "," + (ys[i + 1] - d * tangents[i + 1])
                   + " " + xs[i + 1] + "," + ys[i + 1]

            if (i === n - 2 || pluggedIn(sampleStates[i + 2]) !== power) {
                const key = power ? "power" : "battery"

                lines[key] += run
                areas[key] += run + "L" + xs[i + 1] + "," + height
                              + "L" + xs[runStart] + "," + height + "Z"
                runStart = i + 1
            }
        }

        return {
            count: n,
            xs: xs,
            ys: ys,
            tangents: tangents,
            states: sampleStates,
            lines: lines,
            areas: areas
        }
    }

    // The point on the curve at `x`, with the state of its segment.
    function pointAt(x) {
        const c = curve

        if (c.count < 2)
            return null

        x = Math.max(c.xs[0], Math.min(x, c.xs[c.count - 1]))

        let i = 0

        while (i < c.count - 2 && c.xs[i + 1] < x)
            ++i

        // The segments are cubic Hermite curves with x linear in t.
        const h = c.xs[i + 1] - c.xs[i]
        const t = h > 0 ? (x - c.xs[i]) / h : 0
        const t2 = t * t
        const t3 = t2 * t
        const y = (2 * t3 - 3 * t2 + 1) * c.ys[i] + (t3 - 2 * t2 + t) * h * c.tangents[i]
                  + (-2 * t3 + 3 * t2) * c.ys[i + 1] + (t3 - t2) * h * c.tangents[i + 1]

        return {
            x: x,
            y: y,
            state: c.states[i + 1]
        }
    }

    function xFor(time) {
        return (time - start) / duration * plot.width
    }

    function yFor(value) {
        return plot.height - Math.min(value, yMax) / yMax * plot.height
    }

    // Draws the curve in from the left once, when the first samples arrive.
    property real reveal: 0

    onCurveChanged: {
        if (curve.count > 1 && reveal === 0 && !revealAnimation.running)
            revealAnimation.start()
    }

    NumberAnimation {
        id: revealAnimation
        target: control
        property: "reveal"
        to: 1
        duration: 700
        easing.type: Easing.OutCubic
    }

    component Marker: Item {
        property color color

        width: 0
        height: 0

        // A soft halo, then a ring in the card's colour, so the dot stays
        // clear of the curve and the grid lines it sits on.
        Rectangle {
            anchors.centerIn: parent
            width: 18
            height: width
            radius: width / 2
            color: control.tint(parent.color, 0.18)
        }

        Rectangle {
            anchors.centerIn: parent
            width: 12
            height: width
            radius: width / 2
            color: control.surfaceColor
        }

        Rectangle {
            anchors.centerIn: parent
            width: 8
            height: width
            radius: width / 2
            color: parent.color
        }
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

    // The widest time label, e.g. "12:00 pm".
    TextMetrics {
        id: timeLabelMetrics
        font: control.labelFont
        text: new Date(2000, 0, 1, 12, 0).toLocaleTimeString(Qt.locale(), Locale.ShortFormat)
    }

    TextMetrics {
        id: digitMetrics
        font: control.labelFont
        text: "0123456789"
    }

    // The legend, and the reading under the pointer in its place while hovering.
    Item {
        id: header

        // Text in the header sits on one baseline that centres the digits.
        readonly property real textBaseline: Math.round((height + digitMetrics.tightBoundingRect.height) / 2
                                                        * FishUI.Dpi.ratio) / FishUI.Dpi.ratio

        width: control.width
        height: Math.ceil(labelMetrics.height) + Metrics.s
    }

    Row {
        id: legend
        spacing: Metrics.m
        opacity: readout.visible ? 0 : 1

        Behavior on opacity {
            NumberAnimation { duration: 150 }
        }

        Repeater {
            model: [
                { color: control.batteryColor, text: qsTr("On battery") },
                { color: control.powerColor, text: qsTr("Plugged in") }
            ]

            Row {
                spacing: Metrics.xs + Metrics.xxs

                // Level with the middle of the lowercase letters.
                FishUI.RoundedRectangle {
                    y: header.textBaseline - labelMetrics.xHeight / 2 - height / 2
                    width: 10
                    height: 3
                    radius: height / 2
                    continuousCorners: false
                    color: modelData.color
                }

                FishUI.Label {
                    y: header.textBaseline - baselineOffset
                    text: modelData.text
                    font: control.labelFont
                    color: FishUI.Theme.disabledTextColor
                    renderType: Text.NativeRendering
                }
            }
        }
    }

    Item {
        id: plot
        // Half a label above the top line, so its label is not cut off.
        y: header.height + Metrics.s + Math.ceil(labelMetrics.height / 2)
        width: control.width - Math.ceil(yLabelMetrics.advanceWidth) - Metrics.m
        // One label line below for the time.
        height: control.height - y - Math.ceil(labelMetrics.height) - Metrics.s

        Repeater {
            model: Math.floor(control.yMax / control.yStep) + 1

            FishUI.RoundedRectangle {
                id: gridLine

                readonly property real value: index * control.yStep

                // On the device pixel grid, so the label centres on the line as drawn.
                y: Math.round((control.yFor(value) - height / 2) * FishUI.Dpi.ratio) / FishUI.Dpi.ratio
                width: plot.width
                height: 1 / FishUI.Dpi.ratio
                continuousCorners: false
                color: index === 0 ? control.baselineColor : control.gridColor

                // The digits, not the line box, centred on the line.
                FishUI.Label {
                    anchors.left: parent.right
                    anchors.leftMargin: Metrics.m
                    y: parent.height / 2 + digitMetrics.tightBoundingRect.height / 2 - baselineOffset
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

            Item {
                id: tick

                readonly property real time: control.ticks.first + index * control.ticks.step
                readonly property date date: new Date(time * 1000)
                readonly property bool dayStart: date.getHours() === 0 && date.getMinutes() === 0

                x: control.xFor(time)
                height: plot.height

                // Only a new day gets a line; the hours are read off the labels.
                FishUI.RoundedRectangle {
                    visible: tick.dayStart
                    x: -width / 2
                    width: 1 / FishUI.Dpi.ratio
                    height: plot.height
                    continuousCorners: false
                    color: control.gridColor
                }

                FishUI.Label {
                    // Centred under the tick, but kept inside the graph at either end.
                    x: Math.max(-tick.x, Math.min(control.width - plot.x - tick.x - width, -width / 2))
                    anchors.top: parent.bottom
                    anchors.topMargin: Metrics.s
                    text: tick.dayStart ? tick.date.toLocaleDateString(Qt.locale(), "ddd")
                                        : tick.date.toLocaleTimeString(Qt.locale(), Locale.ShortFormat)
                    font: control.labelFont
                    color: tick.dayStart ? FishUI.Theme.textColor : FishUI.Theme.disabledTextColor
                    renderType: Text.NativeRendering
                }
            }
        }

        // Clips the curve only sideways, and only while it is drawn in.
        Item {
            readonly property real overhang: 8

            x: -overhang
            y: -overhang
            width: (plot.width + overhang * 2) * control.reveal
            height: plot.height + overhang * 2
            clip: control.reveal < 1

            Shape {
                x: parent.overhang
                y: parent.overhang
                width: plot.width
                height: plot.height
                preferredRendererType: Shape.CurveRenderer
                // Qt 6.8's synchronous curve renderer skips a path changed twice before a
                // frame, as a layout settling does, and keeps drawing the first one.
                asynchronous: true

                ShapePath {
                    strokeColor: "transparent"
                    strokeWidth: -1
                    fillGradient: LinearGradient {
                        x1: 0
                        y1: 0
                        x2: 0
                        y2: plot.height
                        GradientStop { position: 0; color: control.tint(control.batteryColor, control.areaOpacity) }
                        GradientStop { position: 1; color: control.tint(control.batteryColor, 0) }
                    }

                    PathSvg {
                        path: control.curve.count > 1 ? control.curve.areas.battery : ""
                    }
                }

                ShapePath {
                    strokeColor: "transparent"
                    strokeWidth: -1
                    fillGradient: LinearGradient {
                        x1: 0
                        y1: 0
                        x2: 0
                        y2: plot.height
                        GradientStop { position: 0; color: control.tint(control.powerColor, control.areaOpacity) }
                        GradientStop { position: 1; color: control.tint(control.powerColor, 0) }
                    }

                    PathSvg {
                        path: control.curve.count > 1 ? control.curve.areas.power : ""
                    }
                }

                ShapePath {
                    strokeColor: control.batteryColor
                    strokeWidth: 2
                    fillColor: "transparent"
                    joinStyle: ShapePath.RoundJoin
                    capStyle: ShapePath.RoundCap

                    PathSvg {
                        path: control.curve.count > 1 ? control.curve.lines.battery : ""
                    }
                }

                ShapePath {
                    strokeColor: control.powerColor
                    strokeWidth: 2
                    fillColor: "transparent"
                    joinStyle: ShapePath.RoundJoin
                    capStyle: ShapePath.RoundCap

                    PathSvg {
                        path: control.curve.count > 1 ? control.curve.lines.power : ""
                    }
                }
            }
        }

        HoverHandler {
            id: hover
        }

        // Follows the pointer along the curve.
        Item {
            id: probe

            readonly property var point: hover.hovered ? control.pointAt(hover.point.position.x) : null

            anchors.fill: parent
            visible: point !== null && control.reveal >= 1

            FishUI.RoundedRectangle {
                x: probe.point ? probe.point.x - width / 2 : 0
                y: header.height - plot.y
                width: 1 / FishUI.Dpi.ratio
                height: plot.height - y
                continuousCorners: false
                color: FishUI.Theme.darkMode ? Qt.rgba(1, 1, 1, 0.28) : Qt.rgba(0, 0, 0, 0.2)
            }

            Marker {
                x: probe.point ? probe.point.x : 0
                y: probe.point ? probe.point.y : 0
                color: probe.point && control.pluggedIn(probe.point.state) ? control.powerColor
                                                                           : control.batteryColor
            }
        }
    }

    Item {
        id: readout

        readonly property var point: probe.point

        visible: probe.visible
        width: readoutRow.implicitWidth + Metrics.s * 2
        height: header.height
        x: point ? Math.max(0, Math.min(control.width - width, plot.x + point.x - width / 2)) : 0

        FishUI.RoundedRectangle {
            anchors.fill: parent
            radius: height / 2
            continuousCorners: false
            color: FishUI.Theme.darkMode ? FishUI.Theme.alternateBackgroundColor : FishUI.Theme.backgroundColor
            border.width: 1 / FishUI.Dpi.ratio
            border.color: control.tint(FishUI.Theme.textColor, FishUI.Theme.darkMode ? 0.1 : 0.08)
        }

        Row {
            id: readoutRow
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: Metrics.xs + Metrics.xxs

            FishUI.Label {
                id: valueLabel
                y: header.textBaseline - baselineOffset
                text: {
                    if (!readout.point)
                        return ""

                    const value = (plot.height - readout.point.y) / plot.height * control.yMax
                    return value.toFixed(control.decimals) + control.yUnits
                }
                font.family: control.labelFont.family
                font.pointSize: control.labelFont.pointSize
                font.weight: Font.DemiBold
                color: FishUI.Theme.textColor
                renderType: Text.NativeRendering
            }

            FishUI.Label {
                anchors.baseline: valueLabel.baseline
                text: {
                    if (!readout.point)
                        return ""

                    const date = new Date((control.start + readout.point.x / plot.width * control.duration) * 1000)
                    let time = date.toLocaleTimeString(Qt.locale(), Locale.ShortFormat)

                    if (control.duration > 86400)
                        time = date.toLocaleDateString(Qt.locale(), "ddd") + " " + time

                    const state = control.stateText(readout.point.state)
                    return state.length > 0 ? state + " · " + time : time
                }
                font: control.labelFont
                color: FishUI.Theme.disabledTextColor
                renderType: Text.NativeRendering
            }
        }
    }
}
