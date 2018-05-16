import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

GridLayout {

    id: configTop

//    header: Label {
//        text: qsTr("Configuration")
//        font.pixelSize: Qt.application.font.pixelSize * 2
//        padding: 10
//    }

    Button {
        Layout.fillWidth: true
        Layout.row: 2
        Layout.column: 1

        Layout.columnSpan: 3

        id: btnDisconnect

        text: "Disconnect"

        onClicked: {
            backend.disconnectFromService()
        }
    }

///'


    RatesList {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.row: 1
        Layout.column: 1

        model: mainRatesModel

        headerLabel: "Roll / Pitch rates"
    }

    ToolSeparator {
        Layout.fillHeight: true
        Layout.row: 1
        Layout.column: 2
    }

    RatesList {
        Layout.fillWidth: true
        Layout.fillHeight: true
        Layout.row: 1
        Layout.column: 3

        model: yawRatesModel

        headerLabel: "Yaw rates"
    }

}
