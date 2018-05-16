import QtQuick 2.9
import QtQuick.Controls 2.3

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Tabs")

    StatusMessage {
        id: statusMessage

        anchors.right: parent.right
        anchors.top: parent.top
        anchors.margins: 10

        z: 100
    }

    SwipeView {
        id: mainSwipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex
        onCurrentIndexChanged: charts.showed = (currentItem.objectName === "chartsPage")

        property bool deviceMessage: backend.showScanner
        onDeviceMessageChanged: {
            if (backend.showScanner) {
                btScanner.visible = true;
                mainSwipeView.opacity = 0.5
            } else {
                btScanner.visible = false
                btScanner.btScanning = false
                mainSwipeView.opacity = 1
            }
        }

        ConfigurationForm {
        }

        ChartForm {
            id: charts
            objectName: "chartsPage"

            showed: false
        }

        // Debug
//        Component.onCompleted: currentIndex = 1
    }

    BtScanner {
        id: btScanner

        height: parent.height / 2
        width: parent.width / 2

        anchors.centerIn: parent
        visible: false
    }

    footer: TabBar {
        id: tabBar
        currentIndex: mainSwipeView.currentIndex

        TabButton {
            text: qsTr("Configuration")
        }
        TabButton {
            text: qsTr("Charts")
        }
    }
}
