#ifndef DEVICEBTSELECT_H
#define DEVICEBTSELECT_H

#include <QtBluetooth/qbluetoothglobal.h>
#include <QtBluetooth/QBluetoothDeviceInfo>
#include <QtBluetooth/QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/QBluetoothLocalDevice>
#include <QtBluetooth/QBluetoothSocket>

#include <QListView>
#include <QDialog>
#include <QPushButton>
#include <QListWidget>
#include <QMessageBox>

class DeviceBTSelect : public QDialog
{
    Q_OBJECT

public:


    DeviceBTSelect(QWidget *parent = 0);
    ~DeviceBTSelect();

    static bool checkBTEnabled(QWidget *parent);
    bool connectDevice();

    void disconnectDevice();

private:
    QBluetoothSocket                *btSocket;

    QBluetoothDeviceDiscoveryAgent  m_discoveryAgent;
    QBluetoothLocalDevice           localDevice;
    QListWidget                     *devicesList;

    QMap<QListWidgetItem *, QBluetoothDeviceInfo> m_discoveredDevices;
    QBluetoothDeviceInfo            m_deviceInfo;

    bool                            isConnected;

    void closeEvent(QCloseEvent *event);
    void startScan();

private slots:
    void socketDisconnected();
    void socketConnected();
    void discoveryFinish();
    void addDevice(const QBluetoothDeviceInfo &info);
    void itemActivated(QListWidgetItem *item);

    void socketError(QBluetoothSocket::SocketError error);
};

#endif // DEVICEBTSELECT_H
