import QtQuick 2.9
import QtQuick.Controls 2.3

ListView {

    id: ratesList
    anchors.margins: 10

    property string headerLabel

    header: Label {
        text: headerLabel
        font.pixelSize: Qt.application.font.pixelSize * 2
        padding: 10

        anchors.horizontalCenter: parent.horizontalCenter
    }

    delegate:
        Row {
            padding: 10
            spacing: 10

            anchors.horizontalCenter: parent.horizontalCenter

//            property real rValue: rateValue

            TextField {

                id: rateField
                validator: DoubleValidator { decimals: 10; bottom: 0 }
                placeholderText: qsTr("SetValue")

                // Just to get small numbers, not long doubles
                text: rateValue.toFixed(5)

                inputMethodHints: Qt.ImhFormattedNumbersOnly

                onTextChanged: rateValue = text
            }

            Label {
                text: rateName
                color: rateField.color
                height: rateField.height

                font.pixelSize: Qt.application.font.pixelSize * 1.5
                padding: 10
            }
        }

    footer: Button {
        text: "Send rates"
        anchors.horizontalCenter: parent.horizontalCenter

        onClicked: {
            model.applyRates()
        }
    }

    focus: true
}
