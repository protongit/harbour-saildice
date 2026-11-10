import QtQuick 2.2
import Sailfish.Silica 1.0

Item {
    id: root

    property double timestamp: 0
    property string expression: ""
    property int total: 0
    property var dice: []

    function contrastingText(value) {
        var hex = value.toString().replace("#", "")
        if (hex.length !== 6)
            return "#f5f2ec"
        var red = parseInt(hex.substr(0, 2), 16)
        var green = parseInt(hex.substr(2, 2), 16)
        var blue = parseInt(hex.substr(4, 2), 16)
        var luminance = 0.2126 * red + 0.7152 * green + 0.0722 * blue
        return luminance > 140 ? "#171717" : "#f5f2ec"
    }

    height: content.height + Theme.paddingMedium

    Column {
        id: content
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        spacing: Theme.paddingSmall

        Row {
            width: parent.width
            spacing: Theme.paddingMedium

            Label {
                id: timeLabel
                text: Qt.formatDateTime(new Date(root.timestamp), "hh:mm:ss")
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
            }
            Label {
                width: parent.width - timeLabel.width - totalLabel.width - 2 * parent.spacing
                text: root.expression
                color: Theme.secondaryColor
                font.pixelSize: Theme.fontSizeSmall
                elide: Text.ElideRight
            }
            Label {
                id: totalLabel
                text: qsTr("Total %1").arg(root.total)
                color: Theme.highlightColor
                font.pixelSize: Theme.fontSizeSmall
            }
        }

        Flow {
            width: parent.width
            spacing: Theme.paddingSmall

            Repeater {
                model: root.dice
                delegate: Rectangle {
                    width: Theme.fontSizeMedium * 2.1
                    height: Theme.fontSizeMedium * 1.45
                    radius: Theme.paddingSmall / 2
                    color: modelData.color
                    border.color: Theme.rgba(Theme.primaryColor, 0.2)
                    border.width: 1

                    Label {
                        anchors.centerIn: parent
                        text: modelData.value
                        color: root.contrastingText(modelData.color)
                        font.pixelSize: Theme.fontSizeSmall
                        font.bold: true
                    }
                }
            }
        }
    }
}
