import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.3

GridLayout {

    id: chartsTop

    property bool showed: true
    onShowedChanged: print("chartView: ", showed)

    ScopeView {
        id: rollScope
        plotModel: rollPlotModel

        chartName: "Roll"

        Layout.row: 1
        Layout.column: 1

        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    ScopeView {
        id: pitchScope
        plotModel: pitchPlotModel

        chartName: "Pitch"

        Layout.row: 1
        Layout.column: 2

        Layout.fillHeight: true
        Layout.fillWidth: true
    }

    Timer {
        id: refreshTimer
        interval: 1 / 30 * 1000 // 30 Hz
        running: chartsTop.showed && dataEnableSw.checked
        repeat: true
        onTriggered: {
            rollScope.updateData()
            pitchScope.updateData()
        }
    }

    Pane {
        Layout.row: 2
        Layout.column: 1

        Layout.columnSpan: 2
//        Layout.rowSpan: 2

//        Layout.fillHeight: true
        Layout.fillWidth: true

        Row {
            id: buttonsPanel

            anchors.centerIn: parent
//            anchors.margins: 10

            Switch {
                id: dataEnableSw
                text: "DataEn"

                scale: 1.2
                onCheckedChanged: {
                    backend.toggleChartState( checked )
                }
            }

            ToolSeparator {
                anchors.leftMargin: 10
                anchors.rightMargin: 10
            }


            ComboBox {
                id: pointCount
                model: [1000, 2000, 5000, 10000, 20000, 50000, 100000]

                onCurrentTextChanged: {
                    backend.updatePlotPointCount( currentText );
                    rollScope.updateData()
                    pitchScope.updateData()
                }

                Component.onCompleted: {
                    backend.updatePlotPointCount( currentText );
                }
            }
        }
    }
}
