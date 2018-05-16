import QtQuick 2.9
import QtQuick.Controls 2.3

Pane {
    id: pane

    SwipeView {
        id: view
        currentIndex: 1
        anchors.fill: parent

        Repeater {
            model: 3

            Pane {
                id: paneView
                width: view.width
                height: view.height

                Column {
                    spacing: 10
                    width: parent.width

                    Label {
                        width: parent.width
                        wrapMode: Label.Wrap
                        horizontalAlignment: Qt.AlignHCenter
                        text: "SwipeView provides a navigation model that simplifies horizontal paged scrolling. "
                        + "The page indicator on the bottom shows which is the presently active page."
                    }

                    Rectangle {
                        width: 50
                        height: 50
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

                    Rectangle {
                        width: 50
                        height: 50
                        color: "red"
                        anchors.horizontalCenter: parent.horizontalCenter
                    }

//                    Image {
//                        width: 50
//                        height: 50
//                        source: "/images/arrows.png"
//                        anchors.horizontalCenter: parent.horizontalCenter
//                        anchors.centerIn: parent.parent
//                    }
                }
            }
        }
    }

    PageIndicator {
        count: view.count
        currentIndex: view.currentIndex
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
