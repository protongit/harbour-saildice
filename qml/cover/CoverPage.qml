import QtQuick 2.2
import Sailfish.Silica 1.0

CoverBackground {
    id: cover
    property string lastRoll: qsTr("No rolls yet")

    signal rollRequested()

    Column {
        anchors.centerIn: parent
        width: parent.width - 2 * Theme.paddingLarge
        spacing: Theme.paddingMedium

        Label {
            width: parent.width
            text: qsTr("SailDice")
            color: Theme.highlightColor
            font.pixelSize: Theme.fontSizeLarge
            horizontalAlignment: Text.AlignHCenter
        }
        Label {
            width: parent.width
            text: cover.lastRoll
            color: Theme.primaryColor
            font.pixelSize: Theme.fontSizeSmall
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            maximumLineCount: 3
            elide: Text.ElideRight
        }
    }

    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-m-refresh"
            onTriggered: cover.rollRequested()
        }
    }
}
