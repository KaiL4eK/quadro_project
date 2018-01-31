#include <QGridLayout>

#include "devicebtselect.h"

QT_BEGIN_NAMESPACE

DeviceBTSelect::DeviceBTSelect(QWidget *parent)
:   QDialog(parent)
{
    devicesList = new QListWidget();

    connect( &m_discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
             this, SLOT(addDevice(QBluetoothDeviceInfo)) );

    connect( &m_discoveryAgent, SIGNAL(finished()),
             this, SLOT(discoveryFinish()) );

    connect( devicesList, SIGNAL(itemActivated(QListWidgetItem*)),
             this, SLOT(itemActivated(QListWidgetItem*)) );

    QGridLayout *mainLayout = new QGridLayout();
    mainLayout->addWidget(devicesList, 0, 0);

    setLayout( mainLayout );
}

DeviceBTSelect::~DeviceBTSelect()
{

}

bool DeviceBTSelect::checkBTEnabled( QWidget *parent )
{
    if ( QBluetoothLocalDevice().hostMode() == QBluetoothLocalDevice::HostPoweredOff )
    {
        QMessageBox::information( parent, "Bluetooth power", "Check Bluetooth is powered on. Exit" );
        return false;
    }

    return true;
}

void DeviceBTSelect::startScan()
{
    if ( m_discoveryAgent.isActive() )
        m_discoveryAgent.stop();

    m_discoveryAgent.start();
}

/** Device found **/
void DeviceBTSelect::addDevice(const QBluetoothDeviceInfo &info)
{
    QString label = QString("%1 %2").arg(info.address().toString()).arg(info.name());
    QList<QListWidgetItem *> items = devicesList->findItems(label, Qt::MatchExactly);
    if (items.empty())
    {
        QListWidgetItem *item = new QListWidgetItem(label);
        QBluetoothLocalDevice::Pairing pairingStatus = localDevice.pairingStatus(info.address());
        if (pairingStatus == QBluetoothLocalDevice::Paired || pairingStatus == QBluetoothLocalDevice::AuthorizedPaired )
        {
            item->setTextColor(QColor(Qt::green));
            m_discoveredDevices.insert(item, info);
        } else
            item->setTextColor(QColor(Qt::black));

        devicesList->addItem(item);
    }
}

void DeviceBTSelect::itemActivated(QListWidgetItem *item)
{
//    QString text = item->text();

//    int index = text.indexOf(' ');

//    if (index == -1)
//        return;

//    QBluetoothAddress address(text.left(index));
//    QString name(text.mid(index + 1));

//    qDebug() << name << address.toString();

    if ( item->textColor() == QColor(Qt::black) )
    {
//        localDevice.requestPairing( address, QBluetoothLocalDevice::Paired );
        return;
    }

    m_deviceInfo = m_discoveredDevices.value(item);
    m_discoveryAgent.stop();

    accept();
}

void DeviceBTSelect::closeEvent(QCloseEvent *event)
{
    m_discoveryAgent.stop();

    reject();
    QWidget::closeEvent(event);
}

bool DeviceBTSelect::connectDevice()
{
    startScan();

    qDebug() << "before exec";
    if ( exec() == QDialog::Accepted )
    {
        QBluetoothDeviceInfo deviceInfo = m_deviceInfo;
//        nameLineEdit->setText(deviceInfo.name());

        btSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
        qDebug() << "Socket created";
        btSocket->connectToService(deviceInfo.address(), deviceInfo.deviceUuid());

        qDebug() << "connectToService done";
//        connectStatusLineEdit->setText("Wait for connect");

        connect(btSocket, SIGNAL(connected()), this, SLOT(socketConnected()) );

        connect(btSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()) );

        connect(btSocket, SIGNAL(error(QBluetoothSocket::SocketError)),
                this,     SLOT(socketError(QBluetoothSocket::SocketError)));

//        connect(btSocket, &QBluetoothSocket::readyRead,
//                this, &ControlWindow::btSocketReadyRead);

        return( true );
    }

    return( false );
}

void DeviceBTSelect::disconnectDevice()
{
    qDebug() << "Called:" << __FUNCTION__;
    btSocket->disconnectFromService();
}

void DeviceBTSelect::socketDisconnected()
{
    qDebug() << "Called:" << __FUNCTION__;
    isConnected = false;
    delete btSocket;
}

void DeviceBTSelect::socketConnected()
{
    qDebug() << "Called:" << __FUNCTION__;
    isConnected = true;
}

void DeviceBTSelect::socketError(QBluetoothSocket::SocketError error)
{
    qDebug() << "Called:" << __FUNCTION__;
    qDebug() << btSocket->errorString();

//    btSocket->
    // OperationError = 19
}

void DeviceBTSelect::discoveryFinish()
{
    qDebug() << "Called:" << __FUNCTION__;

    if ( devicesList->count() == 0 )
    {
        reject();
    }
}
