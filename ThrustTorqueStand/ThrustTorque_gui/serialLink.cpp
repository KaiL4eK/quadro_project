#include "serialLink.h"
#include <QCoreApplication>
#include <QDebug>

#include <serial_protocol.h>

SerialLink::SerialLink(QString sName, QObject *parent)
    : QObject( parent ), serialName( sName )
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

void SerialLink::processStartStopMotorCommand(bool startFlag, quint8 speed)
{
    bool result = false;
    if ( startFlag )
    {
        qDebug() << "Send command to start motors with speed " + QString::number(speed);
        result = processMotorStartCommand(speed);
    } else {
        qDebug() << "Send command to stop motors";
        result = processMotorStopCommand();
    }
    isRunning = result;  // Can`t send == serial error
    emit sendMotorStartStopFinished(result);
}

void SerialLink::clearDataBase()
{
    ma_thrustData.clear();
    ma_torqueData.clear();
    ma_currentData.clear();
    ma_speedData.clear();
    ma_timeData.clear();
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

bool SerialLink::processMotorStartCommand(quint8 speed)
{
    QByteArray command;
    command.append(PARAMETER_PREFIX);
    command.append(PARAM_MOTOR_POWER);
    command.append(speed);
    command.append(COMMAND_PREFIX);
    command.append(CMD_MOTOR_START);
    if ( serial->write( command ) < 0 )
    {
        emit error( "Failed to send motor start command", serial->error() );
        return( false );
    }
    serial->flush();

    receiveData = true;
    clearDataBase();

    return( true );
}

bool SerialLink::processMotorStopCommand()
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
            if ( frame.data[0] == RESP_ENDDATA )
                receiveData = false;
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
             << buffer.time*10.0f << "\t"
             << buffer.speed << "\t" << buffer.current;

    ma_thrustData[current_plot].push_back( buffer.thrust );
    ma_torqueData[current_plot].push_back( buffer.torque );
    ma_currentData[current_plot].push_back( buffer.current );
    ma_speedData[current_plot].push_back( buffer.speed );
    ma_timeData[current_plot].push_back( buffer.time*10.0f ); // milliseconds

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
