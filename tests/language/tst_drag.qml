import QtQuick
import QtTest
import FishUI as FishUI
import "../../src/qml/Language" as LanguageUI

TestCase {
    id: testCase
    name: "PreferredLanguageDragging"
    when: windowShown
    visible: true
    width: 520
    height: 440
    property var view

    QtObject {
        id: backend
        property var preferredLanguages: []
        property bool saving: false
        property int moveCount: 0
        function moveLanguage(from, to) {
            var rows = preferredLanguages.slice()
            rows.splice(to, 0, rows.splice(from, 1)[0])
            preferredLanguages = rows
            moveCount++
        }
    }

    Component {
        id: listComponent
        LanguageUI.PreferredLanguageList {
            x: 20
            y: 20
            width: 480
            height: implicitHeight
            language: backend
        }
    }

    function findVisual(item, name) {
        if (item.objectName === name)
            return item
        for (var i = 0; i < item.children.length; i++) {
            var found = findVisual(item.children[i], name)
            if (found)
                return found
        }
        return null
    }

    function init() {
        FishUI.Theme.fontSize = 10
        backend.preferredLanguages = [{ code: "en_US", name: "English", description: "English (United States)" },
                                      { code: "ja_JP", name: "日本語", description: "Japanese" },
                                      { code: "de_DE", name: "Deutsch", description: "German" }]
        backend.moveCount = 0
        view = createTemporaryObject(listComponent, testCase)
        verify(view)
        waitForRendering(view)
    }

    function startDrag(index) {
        var handle = findVisual(view, "languageDragHandle_" + index)
        verify(handle)
        var point = handle.mapToItem(view, handle.width / 2, handle.height / 2)
        mousePress(view, point.x, point.y)
        mouseMove(view, point.x, point.y + 16)
        tryCompare(view, "dragging", true)
        return point
    }

    function test_moveDown() {
        var point = startDrag(0)
        mouseMove(view, point.x, view.rowHeight * 2.5)
        compare(view.targetIndex, 2)
        compare(backend.moveCount, 0)
        compare(findVisual(view, "languageDragPreview"), null)
        mouseRelease(view, point.x, view.rowHeight * 2.5)
        compare(backend.moveCount, 1)
        compare(backend.preferredLanguages[2].code, "en_US")
        compare(view.dragging, false)
    }

    function test_moveUp() {
        var point = startDrag(2)
        mouseMove(view, point.x, view.rowHeight / 2)
        mouseRelease(view, point.x, view.rowHeight / 2)
        compare(backend.moveCount, 1)
        compare(backend.preferredLanguages[0].code, "de_DE")
    }

    function test_escapeCancels() {
        var point = startDrag(0)
        mouseMove(view, point.x, view.rowHeight * 2.5)
        keyClick(Qt.Key_Escape)
        compare(view.dragging, false)
        mouseRelease(view, point.x, view.rowHeight * 2.5)
        compare(backend.moveCount, 0)
    }

    function test_outsideCancels() {
        var point = startDrag(0)
        mouseMove(view, -10, view.rowHeight * 2.5)
        mouseRelease(view, -10, view.rowHeight * 2.5)
        compare(backend.moveCount, 0)
    }

    function test_noOpDoesNotSave() {
        var point = startDrag(0)
        mouseMove(view, point.x, point.y)
        mouseRelease(view, point.x, point.y)
        compare(backend.moveCount, 0)
    }

    function test_edgeScrolling() {
        var rows = []
        for (var i = 0; i < 12; i++)
            rows.push({ code: "language" + i, name: "Language " + i, description: "Description " + i })
        backend.preferredLanguages = rows
        waitForRendering(view)
        var point = startDrag(0)
        mouseMove(view, point.x, view.height - 1)
        var list = findVisual(view, "preferredLanguageView")
        tryVerify(function() { return list.contentY > view.rowHeight })
        mouseRelease(view, point.x, view.height - 1)
        compare(backend.moveCount, 1)
        verify(backend.preferredLanguages[0].code !== "language0")
    }
}
