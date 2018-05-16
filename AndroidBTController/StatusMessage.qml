import QtQuick 2.9
import QtQuick.Controls 2.3

Rectangle {
    id: notifyMsgTop

    color: "#202020"

    border.width: 2
    border.color: "#202020"

    width: messageText.width * 1.3
    height: messageText.height * 1.3

    NumberAnimation on opacity { id: disappearing; from: 1.0; to: 0.0; duration: 5000 }

    Label {
        id: messageText
        anchors.centerIn: parent

        font.pixelSize: Qt.application.font.pixelSize * 1.2

        property string deviceMessage: backend.statusMessage
        onDeviceMessageChanged: {
            disappearing.restart()
        }

        text: deviceMessage
    }

}
