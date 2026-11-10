import QtQuick 2.2
import Sailfish.Silica 1.0

Item {
    id: root

    property int dieIndex: 0
    property int dieType: 6
    property string dieColor: "#e8e0d0"
    property var dieTypes: []
    property var palette: []

    signal removeRequested(int index)
    signal typeRequested(int index, int type)
    signal colorRequested(int index, string color)

    height: content.height + Theme.paddingMedium
    opacity: enabled ? 1.0 : 0.55

    Column {
        id: content
        x: Theme.horizontalPageMargin
        width: parent.width - 2 * Theme.horizontalPageMargin
        spacing: Theme.paddingSmall

        Row {
            width: parent.width
            height: Theme.itemSizeExtraSmall
            spacing: Theme.paddingMedium

            Rectangle {
                width: Theme.iconSizeSmall
                height: width
                radius: Theme.paddingSmall
                color: root.dieColor
                border.color: Theme.rgba(Theme.primaryColor, 0.25)
                anchors.verticalCenter: parent.verticalCenter
            }
            Label {
                width: parent.width - deleteButton.width - 2 * parent.spacing - Theme.iconSizeSmall
                height: parent.height
                verticalAlignment: Text.AlignVCenter
                text: qsTr("Die %1 · D%2").arg(root.dieIndex + 1).arg(root.dieType)
                color: Theme.primaryColor
                font.pixelSize: Theme.fontSizeSmall
            }
            IconButton {
                id: deleteButton
                icon.source: "image://theme/icon-m-delete"
                enabled: root.enabled
                onClicked: root.removeRequested(root.dieIndex)
            }
        }

        Row {
            width: parent.width
            spacing: Theme.paddingSmall

            Repeater {
                model: root.dieTypes
                delegate: Rectangle {
                    property bool selected: modelData === root.dieType
                    width: (parent.width - 5 * Theme.paddingSmall) / 6
                    height: Theme.itemSizeExtraSmall * 0.78
                    radius: Theme.paddingSmall
                    color: selected ? Theme.rgba(Theme.highlightColor, 0.28)
                                    : Theme.rgba(Theme.primaryColor, 0.075)
                    border.color: selected ? Theme.highlightColor
                                           : Theme.rgba(Theme.primaryColor, 0.16)
                    border.width: 1

                    Label {
                        anchors.centerIn: parent
                        text: "D" + modelData
                        color: parent.selected ? Theme.highlightColor : Theme.primaryColor
                        font.pixelSize: Theme.fontSizeSmall
                    }
                    MouseArea {
                        anchors.fill: parent
                        enabled: root.enabled
                        onClicked: root.typeRequested(root.dieIndex, modelData)
                    }
                }
            }
        }

        Row {
            width: parent.width
            spacing: Theme.paddingSmall

            Repeater {
                model: root.palette
                delegate: Rectangle {
                    property bool selected: modelData === root.dieColor
                    width: (parent.width - 9 * Theme.paddingSmall) / 10
                    height: width
                    radius: width / 2
                    color: modelData
                    border.color: selected ? Theme.highlightColor
                                           : Theme.rgba(Theme.primaryColor, 0.24)
                    border.width: selected ? 3 : 1

                    MouseArea {
                        anchors.fill: parent
                        enabled: root.enabled
                        onClicked: root.colorRequested(root.dieIndex, modelData)
                    }
                }
            }
        }
    }
}
