import QtQuick 2.2
import Sailfish.Silica 1.0
import harbour.saildice 1.0
import "../components"

Page {
    id: page
    objectName: "mainPage"

    signal rollRecorded(string summary)

    property var dieTypes: [4, 6, 8, 10, 12, 20]
    property var palette: ["#e8e0d0", "#c94b4b", "#d59a32", "#3f9b73", "#3978b8",
                           "#7551a6", "#2d8f96", "#b66b3d", "#8c96a3", "#30343c"]
    property int lastTotal: 0
    property string lastExpression: ""
    property var historyEntries: []

    allowedOrientations: Orientation.Portrait

    ListModel {
        id: diceModel
        ListElement { type: 6; color: "#e8e0d0" }
        ListElement { type: 6; color: "#c94b4b" }
    }

    function openConfig() {
        if (tray.rolling)
            return
        pageStack.push(Qt.resolvedUrl("DiceConfigPage.qml"), {
                           "diceModel": diceModel,
                           "palette": palette,
                           "dieTypes": dieTypes
                       })
    }

    function rollDice() {
        tray.roll()
    }

    function syncTray() {
        var values = []
        for (var i = 0; i < diceModel.count; ++i) {
            var die = diceModel.get(i)
            values.push({ "type": die.type, "color": die.color })
        }
        tray.dice = values
    }

    function expressionFor(results) {
        var order = []
        var counts = {}
        for (var i = 0; i < results.length; ++i) {
            var type = results[i].type
            if (counts[type] === undefined) {
                counts[type] = 0
                order.push(type)
            }
            counts[type] += 1
        }
        var parts = []
        for (var j = 0; j < order.length; ++j) {
            var dieType = order[j]
            parts.push((counts[dieType] > 1 ? counts[dieType] : "") + "D" + dieType)
        }
        return parts.join(" + ")
    }

    function handleRoll(results) {
        var total = 0
        for (var i = 0; i < results.length; ++i)
            total += results[i].value
        lastTotal = total
        lastExpression = expressionFor(results)
        var summary = lastExpression + " · total " + total
        var entry = { "timestamp": Date.now(), "expression": lastExpression,
                      "total": total, "dice": results }
        var entries = [entry].concat(historyEntries)
        historyEntries = entries.slice(0, 50)
        rollRecorded(summary)
    }

    onStatusChanged: {
        if (status === PageStatus.Active)
            syncTray()
    }

    Timer {
        id: initialSync
        interval: 0
        onTriggered: syncTray()
    }
    Component.onCompleted: initialSync.start()

    SilicaFlickable {
        id: content
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: sections.height + 2 * Theme.paddingLarge

        PullDownMenu {
            MenuItem {
                text: qsTr("Configure dice")
                enabled: !tray.rolling
                onClicked: page.openConfig()
            }
        }

        Column {
            id: sections
            width: content.width
            spacing: Theme.paddingSmall

            PageHeader {
                title: qsTr("SailDice")
            }

            Item {
                id: trayPanel
                width: parent.width
                height: Math.min(page.height * 0.38, page.width * 0.75)

                Rectangle {
                    anchors {
                        fill: parent
                        leftMargin: Theme.horizontalPageMargin
                        rightMargin: Theme.horizontalPageMargin
                    }
                    radius: Theme.paddingLarge
                    color: Theme.rgba(Theme.highlightColor, 0.055)
                    border.color: Theme.rgba(Theme.highlightColor, 0.13)
                    border.width: 1
                }

                DiceTray {
                    id: tray
                    anchors.fill: parent
                    onRollFinished: page.handleRoll(results)
                }

                Label {
                    anchors.centerIn: parent
                    visible: diceModel.count === 0
                    text: qsTr("Add a die to get started")
                    color: Theme.secondaryColor
                    font.pixelSize: Theme.fontSizeSmall
                }
            }

            Row {
                id: rollRow
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                height: Theme.itemSizeMedium
                spacing: Theme.paddingMedium

                Label {
                    width: parent.width - launchButton.width - parent.spacing
                    height: parent.height
                    verticalAlignment: Text.AlignVCenter
                    text: lastExpression.length > 0 ? lastExpression + " · total " + lastTotal
                                                    : qsTr("Configure your dice and roll")
                    color: lastExpression.length > 0 ? Theme.highlightColor : Theme.secondaryColor
                    font.pixelSize: lastExpression.length > 0 ? Theme.fontSizeMedium : Theme.fontSizeSmall
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    wrapMode: Text.Wrap
                }
                Button {
                    id: launchButton
                    text: tray.rolling ? qsTr("Rolling…") : qsTr("Roll")
                    enabled: !tray.rolling && diceModel.count > 0
                    onClicked: tray.roll()
                }
            }

            SectionHeader { text: qsTr("Roll history") }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Clear history")
                enabled: historyEntries.length > 0
                onClicked: remorse.execute(qsTr("Clearing history"), function() {
                    historyEntries = []
                })
            }

            Label {
                width: parent.width
                visible: historyEntries.length === 0
                text: qsTr("No rolls yet")
                horizontalAlignment: Text.AlignHCenter
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
            }

            Repeater {
                model: historyEntries
                delegate: HistoryDelegate {
                    width: sections.width
                    timestamp: modelData.timestamp
                    expression: modelData.expression
                    total: modelData.total
                    dice: modelData.dice
                }
            }
        }
    }

    RemorsePopup { id: remorse }
}
