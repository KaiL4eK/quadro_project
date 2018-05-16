#include "backend.h"

Backend::Backend(QObject *parent) :
    QObject(parent), m_statusMessage(QString( "Status message =)" )), m_btSocket(nullptr),
    m_stepRangeX_ms(100)
{
    /* Debug mode */
    m_debugMode = false;
    m_showScanner = !m_debugMode;

    /* Data Models */
    connect(&m_ratesRollPitchModel, SIGNAL(sendRates(QVector<float>&)), this, SLOT(sendPIDMainRates(QVector<float>&)));
    connect(&m_ratesYawModel, SIGNAL(sendRates(QVector<float>&)), this, SLOT(sendPIDYawRates(QVector<float>&)));

    /* Data protocol */
    connect(&m_protocol, SIGNAL(sendData(QByteArray&)), this, SLOT(ll_send_request(QByteArray&)));
    connect(&m_protocol, SIGNAL(updateRates(QVector<float>&,SCORP::RatesType)), this, SLOT(receivePIDMainRates(QVector<float>&,SCORP::RatesType)));
    connect(&m_protocol, SIGNAL(newDataReceived(SCORP::data_package_t&)), this, SLOT(newDataReceived(SCORP::data_package_t&)));
    connect(&m_protocol, SIGNAL(updatePlotPeriod(quint32)), this, SLOT(updatePlotPeriod(quint32)));

    /* Plot Model */
    if ( m_debugMode )
    {
        m_dataSimulatorTimer.setInterval( 5 );
        connect(&m_dataSimulatorTimer, SIGNAL(timeout()), this, SLOT(emulateDataReceive()));

        updatePlotPeriod( 5 );
    }

}


void Backend::connectToService(QString address, QString uuid)
{
    m_service_uuid = QBluetoothUuid( uuid );
    m_device_address = QBluetoothAddress( address );

    qDebug() << "Called " << m_device_address << " " << m_service_uuid;

    m_btSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);

    m_btSocket->connectToService(m_device_address, m_service_uuid);

    connect(m_btSocket, &QBluetoothSocket::connected, this, &Backend::btSocketConnected);
    connect(m_btSocket, &QBluetoothSocket::disconnected, this, &Backend::btSocketDisconnected);
    connect(m_btSocket, static_cast<void(QBluetoothSocket::*)(QBluetoothSocket::SocketError)>(&QBluetoothSocket::error), this, &Backend::btSocketError);
    connect(m_btSocket, &QBluetoothSocket::readyRead, this, &Backend::btSocketReadyRead);

    setStatusMessage( QString( "Device set, wait for connect" ) );
}

/*** Models slots ***/

void Backend::sendPIDMainRates(QVector<float> &rates)
{
    qDebug() << "Send roll/pitch rates";
    m_protocol.sendRates( rates, SCORP::RATES_TYPE_PID_ROLL_PITCH );
}

void Backend::sendPIDYawRates(QVector<float> &rates)
{
    qDebug() << "Send yaw rates";
    m_protocol.sendRates( rates, SCORP::RATES_TYPE_PID_YAW );
}

void Backend::receivePIDMainRates(QVector<float> &rates, SCORP::RatesType type)
{
    qDebug() << "Update rates!";

    if ( type == SCORP::RATES_TYPE_PID_ROLL_PITCH )
    {
        m_ratesRollPitchModel.handleNewRates( rates[0], rates[1], rates[2] );
    }
    else if ( type == SCORP::RATES_TYPE_PID_YAW )
    {
        m_ratesYawModel.handleNewRates( rates[0], rates[1], rates[2] );
    }
}

void Backend::resetConnection()
{
    if ( m_btSocket )
    {
        m_protocol.disconnectSlave();
        m_btSocket->close();
        delete m_btSocket;

        m_btSocket = nullptr;

        setShowScanner( true );
    }
}

void Backend::disconnectFromService()
{
    resetConnection();
}

void Backend::connect2HardService()
{
    connectToService( "20:16:06:07:55:56", "{00001101-0000-1000-8000-00805f9b34fb}" );
}

void Backend::btSocketError(QBluetoothSocket::SocketError error)
{
    qDebug() << "BT Error: " + QString::number(error);

    if ( error == (QBluetoothSocket::SocketError)QAbstractSocket::SocketAddressNotAvailableError )
    {
//        resetConnection();
    }
}

void Backend::btSocketConnected()
{
    qDebug() << "BT Connected";
    setShowScanner( false );
    setStatusMessage( QString( "Device connected" ) );

    m_protocol.connectSlave();
}

void Backend::btSocketDisconnected()
{
    qDebug() << "BT Disconnected";

    resetConnection();
}

void Backend::clearBTInput()
{
    if ( m_btSocket->isOpen() && m_btSocket->bytesAvailable() > 0 )
    {
        // Remove gibberish =)
        m_btSocket->readAll();
    }
}

void Backend::ll_send_request( QByteArray &data )
{
    m_socketMutex.lock();

    m_btSocket->write( data.data(), data.length() );
    m_btSocket->waitForBytesWritten( 1000 );

    m_socketMutex.unlock();
}

void Backend::btSocketReadyRead()
{
    QByteArray data = m_btSocket->readAll();

    m_protocol.processInputData( data );
}


/*** Plot model functions ***/

static float    tri_amplitude   = 0.0;
static qint32   plot_idx        = 0;

void Backend::emulateDataReceive()
{
    SCORP::data_package_t   sim_pack;

    /* Each bunch of points change amplitude */
    if ( plot_idx % 400 == 0 )
    {
        tri_amplitude = qrand() % 10;
    }

    sim_pack.packId = plot_idx;
    sim_pack.roll   = tri_amplitude * sin( plot_idx * 0.1 ) * DATA_FLOAT_MULTIPLIER;
    sim_pack.pitch  = tri_amplitude * cos( plot_idx * 0.1 ) * DATA_FLOAT_MULTIPLIER;

    /* Random package lost (1%) */
    if ( qrand() % 100 > 1 )
        newDataReceived( sim_pack );

    plot_idx++;
}

void Backend::newDataReceived(SCORP::data_package_t &data)
{
//qDebug() << "New data: " + QString::number(data.packId * m_stepRangeX_ms);
    QPointF rollPoint( data.packId * m_stepRangeX_ms, data.roll * 1.0 / DATA_FLOAT_MULTIPLIER );
    m_rollModel.newDataReceived(data.packId, rollPoint);

    QPointF pitchPoint( data.packId * m_stepRangeX_ms, data.pitch * 1.0 / DATA_FLOAT_MULTIPLIER );
    m_pitchModel.newDataReceived(data.packId, pitchPoint);
}

void Backend::updatePlotPeriod(quint32 periodMs)
{
    qDebug() << "Update period " << periodMs;
    m_stepRangeX_ms = periodMs;

    m_pitchModel.setDataPeriod( m_stepRangeX_ms );
    m_rollModel.setDataPeriod( m_stepRangeX_ms );
}

void Backend::updatePlotPointCount(QString pointCountText)
{
    qDebug() << "Update count " << pointCountText;
    quint32 count = pointCountText.toInt();

    m_pitchModel.setRenderPointCount( count );
    m_rollModel.setRenderPointCount( count );
}

void Backend::toggleChartState( bool enabled )
{
    if ( enabled )
    {
        m_rollModel.resetDataCache();
        m_pitchModel.resetDataCache();

        m_protocol.startDataSending();

        if ( m_debugMode )
        {
            tri_amplitude   = 0.0;
            plot_idx        = 0;
            m_dataSimulatorTimer.start();
        }
    }
    else
    {
        m_protocol.stopDataSending();

        if ( m_debugMode )
        {
            m_dataSimulatorTimer.stop();
        }
    }
}

/*** QML properties ***/

bool Backend::showScanner() const
{
    return m_showScanner;
}

void Backend::setShowScanner(bool value)
{
    m_showScanner = value;

    emit showScannerChanged();
}

QString Backend::statusMessage() const
{
    return m_statusMessage;
}

void Backend::setStatusMessage(QString value)
{
    m_statusMessage = value;
    emit statusMessageChanged();
}
