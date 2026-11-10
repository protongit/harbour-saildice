import QtQuick 2.2
import Sailfish.Silica 1.0
import "pages"
import "cover"

ApplicationWindow {
    id: app

    property string lastRoll: qsTr("No rolls yet")

    function rollFromCover() {
        if (pageStack.currentPage && pageStack.currentPage.objectName === "configPage")
            pageStack.pop()
        var current = pageStack.currentPage
        if (current && current.objectName === "mainPage") {
            current.syncTray()
            current.rollDice()
        }
    }

    initialPage: Component {
        MainPage {
            onRollRecorded: app.lastRoll = summary
        }
    }
    cover: Component {
        CoverPage {
            lastRoll: app.lastRoll
            onRollRequested: app.rollFromCover()
        }
    }
    allowedOrientations: Orientation.Portrait
}
