import QtQuick 2.2
import Sailfish.Silica 1.0
import "../components"

Page {
    id: page
    objectName: "configPage"

    property var diceModel
    property var palette: []
    property var dieTypes: []

    function addDie() {
        if (diceModel.count >= 12)
            return
        var type = diceModel.count > 0 ? diceModel.get(diceModel.count - 1).type : 6
        var color = palette[diceModel.count % palette.length]
        diceModel.append({ "type": type, "color": color })
    }

    function removeDie(index) {
        diceModel.remove(index)
    }

    function setDieType(index, type) {
        diceModel.setProperty(index, "type", type)
    }

    function setDieColor(index, color) {
        diceModel.setProperty(index, "color", color)
    }

    SilicaFlickable {
        id: content
        anchors.fill: parent
        clip: true
        contentWidth: width
        contentHeight: sections.height + 2 * Theme.paddingLarge

        Column {
            id: sections
            width: content.width
            spacing: Theme.paddingSmall

            PageHeader { title: qsTr("Configure dice") }

            Label {
                x: Theme.horizontalPageMargin
                width: parent.width - 2 * Theme.horizontalPageMargin
                text: qsTr("Choose the type and color of each die.")
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                wrapMode: Text.Wrap
            }

            Repeater {
                model: diceModel
                delegate: DiceConfigDelegate {
                    width: sections.width
                    dieIndex: index
                    dieType: model.type
                    dieColor: model.color
                    dieTypes: page.dieTypes
                    palette: page.palette
                    onRemoveRequested: page.removeDie(index)
                    onTypeRequested: page.setDieType(index, type)
                    onColorRequested: page.setDieColor(index, color)
                }
            }

            Button {
                anchors.horizontalCenter: parent.horizontalCenter
                text: qsTr("Add die")
                enabled: diceModel.count < 12
                onClicked: page.addDie()
            }
        }
    }
}
