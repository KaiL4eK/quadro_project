import QtQuick 2.9
import QtQuick.Controls 2.3
import QtBluetooth 5.9


Rectangle {
    id: top

    z: 50

    color: "#202020"

    border.width: 2
    border.color: "#202020"
    radius: 5

    property bool             btScanning: false
    onBtScanningChanged: btModel.running = btScanning

    BluetoothDiscoveryModel {
        id: btModel
        running: true
//        uuidFilter: "00001101-0000-1000-8000-00805f9b34fb"
        discoveryMode: BluetoothDiscoveryModel.MinimalServiceDiscovery
        onDiscoveryModeChanged: console.log("Discovery mode: " + discoveryMode)
        onServiceDiscovered: console.log("Found new service " + service.deviceAddress + " " + service.deviceName + " " + service.serviceName);
        onDeviceDiscovered: console.log("New device: " + device)
        onRunningChanged: btScanning = btModel.running
        onErrorChanged: {
                switch (btModel.error) {
                case BluetoothDiscoveryModel.PoweredOffError:
                    console.log("Error: Bluetooth device not turned on"); break;
                case BluetoothDiscoveryModel.InputOutputError:
                    console.log("Error: Bluetooth I/O Error"); break;
                case BluetoothDiscoveryModel.InvalidBluetoothAdapterError:
                    console.log("Error: Invalid Bluetooth Adapter Error"); break;
                case BluetoothDiscoveryModel.NoError:
                    break;
                default:
                    console.log("Error: Unknown Error"); break;
                }
        }
    }

    ListView {
        id: mainList
        width: top.width
        anchors.top: top.top
        anchors.bottom: button.top
        anchors.bottomMargin: 10
        anchors.topMargin: 10
        clip: true

        model: btModel
        delegate: ItemDelegate {

            width: parent.width

            text: deviceName + " / Type: " + service.serviceName + " / Address: " + remoteAddress + " / UUID: " + service.serviceUuid

            onClicked: {
                backend.connectToService( service.deviceAddress, service.serviceUuid )
            }
        }
        focus: true


    }

    Button {
        id: button

        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottomMargin: 5

        height: label.height * 2
        width: (label.width + refreshBtImage.width) * 1.3

        highlighted: btScanning

        Row {
            anchors.centerIn: parent

            Text {
                id: label
                text: "Refresh"
                font.bold: true
                font.pointSize: 20

                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
            }

            Image {
                id: refreshBtImage
                height: label.height
                fillMode: Image.PreserveAspectFit
                source: "images/refresh.png";

                anchors.margins: 10

                RotationAnimation on rotation{
                    id: ranimation
                    target: refreshBtImage
                    easing.type: Easing.InOutBack
                    property: "rotation"
                    from: 0
                    to: -360
                    duration: 2000
                    loops: Animation.Infinite
                    alwaysRunToEnd: true
                    running: btScanning
                }
            }
        }

        onClicked: {
            btScanning = !btScanning
        }
    }

}
