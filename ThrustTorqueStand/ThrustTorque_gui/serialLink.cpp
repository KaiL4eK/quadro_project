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
        qDebug() << "Send command to start motors with speed " +
                    QString::number(speed);
        result = processMotorStartCommand(speed);
    } else {
        qDebug() << "Send command to stop motors";
        result = processMotorStopCommand();
    }
    isRunning = result;  // Can`t send = serial error
    emit sendMotorStartStopFinished(result);
}

void SerialLink::clearDataBase()
{
    thrustList.clear();
    torqueList.clear();
    currentList.clear();
    speedList.clear();
    timeList.clear();
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

    return( waitForResponse() );
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
QThread::msleep( 100 );

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
    if ( serial->write( command ) < 0 )
    {
        emit error( "Failed to send motor stop command", serial->error() );
        return( false );
    }
    serial->flush();

    return( true );
}

bool SerialLink::processReceiveData()
{
    QByteArray frame = receiveNextFrame();
    if ( frame.isEmpty() )
        return( false );

    switch ( frame[0] )
    {
        case RESPONSE_PREFIX:
            if ( frame[1] == RESP_ENDDATA )
                receiveData = false;
            break;
        case DATA_PREFIX:
            if ( receiveData )
            {
                QByteArray frameContent = frame.mid(1, DATA_STAND_FRAME_SIZE);
                parseDataFrame( frameContent );
            }
            break;
        case 0:
        case COMMAND_PREFIX:
        default:
            break;
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

    thrustList[current_plot].push_back( buffer.thrust );
    torqueList[current_plot].push_back( buffer.torque );
    currentList[current_plot].push_back( buffer.current );
    speedList[current_plot].push_back( buffer.speed );
    timeList[current_plot].push_back( buffer.time*10.0f ); // milliseconds

    emit dataReceived();
}

quint8 SerialLink::receiveFrameHead()
{
    while ( serial->bytesAvailable() < 1 )  // Receive frame type
    {
        if ( !serial->waitForReadyRead( 1000 ) )
        {
            if ( serial->error() != QSerialPort::TimeoutError )
                emit error( "Failed to receive frame type [serial error]",
                            serial->error() );
            else
                emit error( "Failed to receive frame type [timeout]",
                            serial->error() );

            return( 0 );
        }
    }
    char frameHead;
    if ( serial->read(&frameHead, 1) < 0 )
    {
        emit error( "Failed to receive frame type [serial error]",
                    serial->error() );
        return( 0 );
    }

    return( frameHead );
}

QByteArray SerialLink::receiveNextFrame()
{
    QByteArray result;
    quint8 type = receiveFrameHead();
    quint8 length = 0;
    switch( type )
    {
        case RESPONSE_PREFIX:
            length = RESPONSE_FRAME_SIZE;
            break;
        case DATA_PREFIX:
            length = DATA_STAND_FRAME_SIZE;
            break;
        case 0:
        case COMMAND_PREFIX:
        default:
            return( result );
    }
    while ( serial->bytesAvailable() < length )
    {
        if ( !serial->waitForReadyRead( 500 ) )
        {
            if ( serial->error() != QSerialPort::TimeoutError )
                emit error( "Failed to receive frame type [serial error]",
                            serial->error() );
            else
                emit error( "Failed to receive frame type [timeout]",
                            serial->error() );
            result.clear();
            return( result );
        }
    }
    result = serial->read(length);
    if ( result.length() != length )
    {
        emit error( "Failed to receive frame content [serial error]",
                    serial->error() );
        result.clear();
    }
    else
    {
        result.insert(0, (char)type);
    }
    return( result );
}

bool SerialLink::waitForResponse()
{
    QByteArray replyBytes = receiveNextFrame();
    if ( replyBytes.isEmpty() )
        return( false );

    if ( replyBytes[0] != RESPONSE_PREFIX || replyBytes[1] != RESP_NOERROR )
    {
        emit error( "Received response with code: " + QString::number(replyBytes[1]),
                    serial->error() );
        return( false );
    }
    qDebug() << "Received " + QString::number(replyBytes[0]) << " " + QString::number(replyBytes[1]);
    return( true );
}
