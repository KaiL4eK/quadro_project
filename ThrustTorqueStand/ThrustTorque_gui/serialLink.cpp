#include "serialLink.h"
#include <QCoreApplication>
#include <QDebug>



SerialLink::SerialLink(QString sName, QObject *parent)
    : QObject( parent ), serialName( sName ), current_plot( -1 ),
      serialSpeed( 460800 )
{

}

SerialLink::~SerialLink()
{
    emit sendConnectionState( false );

    serial->close();
    delete serial;
}

void SerialLink::stopLink()
{
    qDebug() << "Stop link signal emmited";
    processDisconnectCommand();
    QThread::msleep( 100 );
    isRunning = false;
}

bool SerialLink::makeLinkToPort()
{
    serial = new QSerialPort( serialName );
    serial->setBaudRate( serialSpeed );
    serial->setDataBits( QSerialPort::Data8 );
    serial->setParity( QSerialPort::NoParity );
    serial->setStopBits( QSerialPort::OneStop );
    serial->setFlowControl( QSerialPort::NoFlowControl );

    if ( serial->open( QIODevice::ReadWrite ) )
        return( true );

    emit error( "Failed to connect to serial port: " + serial->portName(), serial->error() );
    return( false );
}

void SerialLink::process()
{
    if ( makeLinkToPort() && processConnectCommand() )
    {
        emit setDataSource(ThrustDataIndex, &ma_thrustData, &ma_timeData);
        emit setDataSource(TorqueDataIndex, &ma_torqueData, &ma_timeData);
        emit setDataSource(CurrentDataIndex, &ma_currentData, &ma_timeData);
        emit setDataSource(SpeedDataIndex, &ma_speedData, &ma_timeData);

        emit sendConnectionState( true );
        isRunning = true;
        while( isRunning )
        {
            if ( serial->bytesAvailable() > 0 )
                processReceiveData();
            QCoreApplication::processEvents();
        }
    }
    emit finished();
}

void SerialLink::processStartStopMotorCommand(bool startFlag)
{
    bool result = false;
    if ( startFlag )
    {
        qDebug() << "Send command to start motors";
        result = processMeasureStartCommand();
    } else {
        qDebug() << "Send command to stop motors";
        result = processMeasureStopCommand();
    }
    isRunning = result;  // Can`t send == serial error
}

void SerialLink::resizeDatabase()
{
    emit addNewCurve(ThrustDataIndex);
    emit addNewCurve(TorqueDataIndex);
    emit addNewCurve(CurrentDataIndex);
    emit addNewCurve(SpeedDataIndex);

    ma_thrustData.resize(current_plot+1);
    ma_torqueData.resize(current_plot+1);
    ma_currentData.resize(current_plot+1);
    ma_speedData.resize(current_plot+1);
    ma_timeData.resize(current_plot+1);
}

bool SerialLink::processConnectCommand()
{
//    if ( serial->isOpen() )
//        serial->clear();

    QByteArray command;
    command.append(COMMAND_PREFIX);
    command.append(CMD_CONNECT_CODE);
    if ( serial->write( command ) < 0 )
    {
        emit error( "Failed to send connection command", serial->error() );
        return( false );
    }
    serial->flush();

    return( receiveResponse() );
}

bool SerialLink::processDisconnectCommand()
{
//    if ( serial->isOpen() )
//        serial->clear();

    qDebug() << "Send disconnect cmd";

    QByteArray command;
    command.append(COMMAND_PREFIX);
    command.append(CMD_DISCONNECT_CODE);
    if ( serial->write( command ) < 0 ) {
        emit error( "Failed to send disconnection command", serial->error() );
        return( false );
    }
    serial->flush();

    return( true );
}


void SerialLink::processSetParametersCommand(MeasureParams params)
{
    qDebug() << "Send parameters cmd";

    QByteArray command;
    QDataStream streamRoll( &command, QIODevice::ReadWrite );

    streamRoll << (uint8_t)PARAMETERS_PREFIX;
    streamRoll << params.motor_power_start;
    streamRoll << params.motor_power_stop;
    streamRoll << (uint16_t)(params.time_measure_start_ms/MEASURE_PERIOD_MS);
    streamRoll << (uint16_t)(params.time_measure_ms/MEASURE_PERIOD_MS);
    streamRoll << (uint16_t)(params.time_step_moment_ms/MEASURE_PERIOD_MS);

    if ( command.size() != PARAMETER_FRAME_SIZE + 1 ) {
        emit error( "Incorrect parameters frame size", -1 );
    }

    if ( serial->write( command ) < 0 ) {
        emit error( "Failed to send parameters command", serial->error() );
    }
    serial->flush();

    if( !receiveResponse() ) {
        emit error( "Bad response after parameters set", -9 );
    }

    parametersSent = true;
}

bool SerialLink::processMeasureStartCommand()
{
    if ( !parametersSent ) {
        emit error( "Send parameters first", -7 );
        emit sendDataReceiveFinished();
        return( true ); // Let`s thinks it`s not error =)
    }

    QByteArray command;
    command.append(COMMAND_PREFIX);
    command.append(CMD_MOTOR_START);
    if ( serial->write( command ) < 0 ) {
        emit error( "Failed to send motor start command", serial->error() );
        return( false );
    }
    serial->flush();

    receiveData = true;
    ++current_plot;
    resizeDatabase();

    return( true );
}

bool SerialLink::processMeasureStopCommand()
{
    QByteArray command;
    command.append(COMMAND_PREFIX);
    command.append(CMD_MOTOR_STOP);
    if ( serial->write( command ) < 0 ) {
        emit error( "Failed to send motor stop command", serial->error() );
        return( false );
    }
    serial->flush();

    return( true );
}

bool SerialLink::processReceiveData()
{
    receiveNextFrame();

    switch ( frame.type )
    {
        case RESPONSE_PREFIX:
            if ( frame.data[0] == RESP_ENDDATA ) {
                receiveData = false;
                emit sendDataReceiveFinished();
                qDebug() << "Received data end response";
            }
            break;
        case DATA_PREFIX:
            if ( receiveData )
                parseDataFrame( frame.data );
            break;
        case 0:
        case COMMAND_PREFIX:
        default:
            qDebug() << "Broken serial link frame received";
            return( false );
    }

    return( true );
}

struct serial_data_
{
    quint16  thrust,
             torque,
             speed,
             time;
    qint16   current;
};

QDataStream &operator >>(QDataStream &out, serial_data_ &any)
{
    out >> any.thrust;
    out >> any.torque;
    out >> any.current;
    out >> any.speed;
    out >> any.time;
    return out;
}

void SerialLink::parseDataFrame(QByteArray &frame)
{
    serial_data_ buffer;

    QDataStream streamRoll( &frame, QIODevice::ReadWrite );
    streamRoll >> buffer;

    qDebug() << buffer.thrust << "\t" << buffer.torque << "\t"
             << buffer.time*MEASURE_PERIOD_MS << "\t"
             << buffer.speed << "\t" << buffer.current;

    ma_thrustData[current_plot].push_back( buffer.thrust/10.0f );               // g
    ma_torqueData[current_plot].push_back( buffer.torque );                     // Nm - Now is zero
    ma_currentData[current_plot].push_back( buffer.current*1000.0f/81.8f );     // mA
    ma_speedData[current_plot].push_back( buffer.speed );                       // rpm
    ma_timeData[current_plot].push_back( buffer.time*MEASURE_PERIOD_MS );

    emit dataReceived();
}

void SerialLink::receiveFrameHeader()
{
    frame.clear();

    while ( serial->bytesAvailable() < 1 )  // Receive frame type
    {
        if ( !serial->waitForReadyRead( 1000 ) )
        {
            if ( serial->error() != QSerialPort::TimeoutError )
                emit error( "Failed to receive frame type [serial error]", serial->error() );
            else
                emit error( "Failed to receive frame type [timeout]", serial->error() );

            return;
        }
    }

    if ( serial->read((char *)&frame.type, 1) < 0 ) {
        emit error( "Failed to receive frame type [serial error]", serial->error() );
        frame.type = 0;
    }
}

void SerialLink::receiveNextFrame()
{
    receiveFrameHeader();

    switch( frame.type )
    {
        case RESPONSE_PREFIX:
            frame.length = RESPONSE_FRAME_SIZE;
            break;
        case DATA_PREFIX:
            frame.length = DATA_STAND_FRAME_SIZE;
            break;
        case 0:
        case COMMAND_PREFIX:
        default:
            return;
    }

    while ( serial->bytesAvailable() < frame.length ) {
        if ( !serial->waitForReadyRead( 500 ) )
        {
            if ( serial->error() != QSerialPort::TimeoutError )
                emit error( "Failed to receive frame type [serial error]", serial->error() );
            else
                emit error( "Failed to receive frame type [timeout]", serial->error() );

            frame.type = 0;
            return;
        }
    }

    frame.data += serial->read(frame.length);
    if ( frame.data.length() != frame.length ) {
        emit error( "Failed to receive frame content [serial error]", serial->error() );

        frame.type = 0;
    }
}

bool SerialLink::receiveResponse()
{
    receiveNextFrame();

    if ( frame.type == 0 )
        return( false );

    if ( frame.type != RESPONSE_PREFIX || frame.data[0] != RESP_NOERROR )
    {
        emit error( "Received response with code: " + QString::number(frame.data[0]), serial->error() );
        return( false );
    }
    qDebug() << "Received " + QString::number(frame.type) << " " + QString::number(frame.data[0]);
    return( true );
}
