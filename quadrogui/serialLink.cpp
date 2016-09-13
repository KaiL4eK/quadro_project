#include "serialLink.h"
#include <QCoreApplication>
#include <QDebug>

#include <serial_protocol.h>

SerialLink::SerialLink(QVector<double> *rollDbPtr, QVector<double> *pitchDbPtr,
                       QVector<double> *encRollDbPtr, QVector<double> *encPitchDbPtr,
                       QVector<double> *timeVectPtr , QString sName, QObject *parent)
    : QObject( parent ),
      rollDataList( rollDbPtr ), pitchDataList( pitchDbPtr ),
      encRollDataList( encRollDbPtr ), encPitchDataList( encPitchDbPtr ),
      timeList( timeVectPtr ), serialName( sName ) {}

SerialLink::~SerialLink()
{
    if ( calibrationFlag )
        emit sendCalibrationReady(false);
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

void SerialLink::setDataCommandFlag( bool dataRcvBtnState )
{
    if ( dataRcvBtnState )
    {
        isRunning = processDataStartCommand();
        receiveData = true;
        clearDataBase();
    }
    else
    {
        isRunning = processDataStopCommand();
        receiveData = false;
    }
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

void SerialLink::calibrateSources()
{
    calibrationCounter = 20;
    calibrationFlag = true;
    encoderRollCOffset = encoderPitchCOffset = 0;
    setDataCommandFlag(Qt::Checked);
}

void SerialLink::clearDataBase()
{
    rollDataList->clear();
    pitchDataList->clear();
    encRollDataList->clear();
    encPitchDataList->clear();
    timeList->clear();
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

    return( true );
}

bool SerialLink::processDataStartCommand()
{
    QByteArray command;
    command.append(COMMAND_PREFIX);
    command.append(CMD_DATA_START_CODE);
    if ( serial->write( command ) < 0 )
    {
        emit error( "Failed to send data start command", serial->error() );
        return( false );
    }
    serial->flush();

    return( true );
}

bool SerialLink::processDataStopCommand()
{
    QByteArray command;
    command.append(COMMAND_PREFIX);
    command.append(CMD_DATA_STOP_CODE);
    if ( serial->write( command ) < 0 )
    {
        emit error( "Failed to send data stop command", serial->error() );
        return( false );
    }
    serial->flush();

//    while ( serial->bytesAvailable() > 0 )
//        processReceiveData();

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
                QByteArray frameContent = frame.mid(1, DATA_FULL_FRAME_SIZE);
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
    qint16  rollAngle,
            pitchAngle,
            timeMoment,
            encoderRoll,
            encoderPitch;
    qint8   motor1_power,
            motor2_power,
            motor3_power,
            motor4_power;
};

QDataStream &operator >>(QDataStream &out, serial_data_ &any)
{
    out >> any.rollAngle;
    out >> any.pitchAngle;
    out >> any.timeMoment;
    out >> any.motor1_power;
    out >> any.motor2_power;
    out >> any.motor3_power;
    out >> any.motor4_power;
    out >> any.encoderRoll;
    out >> any.encoderPitch;
    return out;
}

void SerialLink::parseDataFrame(QByteArray &frame)
{
    serial_data_ buffer;

    QDataStream streamRoll( &frame, QIODevice::ReadWrite );
    streamRoll >> buffer;

#define ENCODER_COEFFICIENT 100.0f
#define ANGLES_COEFFICIENT  100.0f

    qDebug() << buffer.rollAngle/ANGLES_COEFFICIENT << "\t" << buffer.pitchAngle/ANGLES_COEFFICIENT << "\t"
             << buffer.timeMoment*10.0f << "\t" << buffer.encoderRoll/ENCODER_COEFFICIENT << "\t"
             << buffer.encoderPitch/ENCODER_COEFFICIENT << "\t"
             << buffer.motor1_power << "\t" << buffer.motor2_power << "\t"
             << buffer.motor3_power << "\t" << buffer.motor4_power;

    emit sendMotorPowers( buffer.motor1_power, buffer.motor2_power, buffer.motor3_power, buffer.motor4_power );

    rollDataList->push_back( buffer.rollAngle/ANGLES_COEFFICIENT );
    pitchDataList->push_back( buffer.pitchAngle/ANGLES_COEFFICIENT );
    encRollDataList->push_back( buffer.encoderRoll/ENCODER_COEFFICIENT + encoderRollCOffset );
    encPitchDataList->push_back( buffer.encoderPitch/ENCODER_COEFFICIENT + encoderPitchCOffset );
    timeList->push_back( buffer.timeMoment*10.0f ); // milliseconds

    if ( calibrationFlag )
    {
        if ( --calibrationCounter == 0 )
        {
            calibrationFlag = false;
            setDataCommandFlag(Qt::Unchecked);
            for ( int i = 0; i < timeList->length(); i++)
            {
                encoderRollCOffset += rollDataList->at(i) - encRollDataList->at(i);
                encoderPitchCOffset += pitchDataList->at(i) - encPitchDataList->at(i);
            }

            encoderRollCOffset /= timeList->length();
            encoderPitchCOffset /= timeList->length();

            qDebug() << QString::number(encoderRollCOffset) << "\t" << QString::number(encoderPitchCOffset) << "\n";
            emit sendCalibrationReady(true);
        }
    }
    else
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
            length = DATA_FULL_FRAME_SIZE;
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
    qDebug() << "Received " + QString::number(replyBytes[1]);
    return( true );
}
