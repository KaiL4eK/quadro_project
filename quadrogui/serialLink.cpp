#include "serialLink.h"
#include <QCoreApplication>
#include <QDebug>

SerialLink::SerialLink(QVector<double> *rollDbPtr, QVector<double> *pitchDbPtr,
                       QVector<double> *encRollDbPtr, QVector<double> *encPitchDbPtr,
                       QVector<double> *timeVectPtr , QString sName, QObject *parent)
    : QObject( parent ),
      rollDataList( rollDbPtr ), pitchDataList( pitchDbPtr ),
      encRollDataList( encRollDbPtr ), encPitchDataList( encPitchDbPtr ),
      timeList( timeVectPtr ), serialName( sName )
{

}

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
    processMotorStopCommand();
    processDataStopCommand();
    isRunning = false;
}

void SerialLink::process()
{
    if ( makeLinkToPort() && processConnectionCommand() )
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
    }
    else
    {
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

bool SerialLink::processConnectionCommand()
{
    if ( serial->isOpen() )
        serial->clear();

    QByteArray command;
    command.append(Command);
    command.append(cmdConnect);
    if ( serial->write( command ) < 0 )
    {
        emit error( "Failed to send connection command", serial->error() );
        return( false );
    }
    serial->flush();

    return( waitForResponse() );
}

bool SerialLink::processDataStartCommand()
{
    QByteArray command;
    command.append(Command);
    command.append(cmdDataStart);
    if ( serial->write( command ) < 0 )
    {
        emit error( "Failed to send data start command", serial->error() );
        return( false );
    }
    serial->flush();

    return( true );
}

bool SerialLink::waitForResponse()
{
    QByteArray replyBytes = receiveNextFrame();
    if ( replyBytes.isEmpty() )
        return( false );

    if ( replyBytes[0] != (char)Response || replyBytes[1] != noErrorResponse )
    {
        emit error( "Received response with code: " + QString::number(replyBytes[1]),
                    serial->error() );
        return( false );
    }
    qDebug() << "Received " + QString::number(replyBytes[1]);
    return( true );
}

bool SerialLink::processDataStopCommand()
{
    QByteArray command;
    command.append(Command);
    command.append(cmdDataStop);
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
    command.append(Parameter);
    command.append(paramMotorStart);
    command.append(speed);
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
    command.append(Parameter);
    command.append(paramMotorStop);
    command.append((char)0);
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
        case Response:
            if ( frame[1] == dataStopResp )
                receiveData = false;
            break;
        case Data:
            if ( receiveData )
            {
                QByteArray frameContent = frame.mid(1, dataFrameSize);
                parseDataFrame( frameContent );
            }
            break;
        case None:
        case Command:
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

#define ANGLE_COEFFICIENT   100.0f
#define ENCODER_COEFFICIENT 100.0f

    qDebug() << buffer.rollAngle/ANGLE_COEFFICIENT << "\t" << buffer.pitchAngle/ANGLE_COEFFICIENT << "\t"
             << buffer.timeMoment*10.0f << "\t" << buffer.encoderRoll/ENCODER_COEFFICIENT << "\t"
             << buffer.encoderPitch/ENCODER_COEFFICIENT << "\t"
             << buffer.motor1_power << "\t" << buffer.motor2_power << "\t"
             << buffer.motor3_power << "\t" << buffer.motor4_power;

    sendMotorPowers( buffer.motor1_power, buffer.motor2_power, buffer.motor3_power, buffer.motor4_power );

    rollDataList->push_back( buffer.rollAngle/ANGLE_COEFFICIENT );
    pitchDataList->push_back( buffer.pitchAngle/ANGLE_COEFFICIENT );
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

bool SerialLink::makeLinkToPort()
{
    initLink();
    if ( serial->open( QIODevice::ReadWrite ) )
        return( true );

    emit error( "Failed to connect to serial port: " + serial->portName(), serial->error() );
    return( false );
}

int SerialLink::initLink()
{
    serial = new QSerialPort( serialName );
    serial->setBaudRate( serialSpeed );
    serial->setDataBits( QSerialPort::Data8 );
    serial->setParity( QSerialPort::NoParity );
    serial->setStopBits( QSerialPort::OneStop );
    serial->setFlowControl( QSerialPort::NoFlowControl );
    return( 0 );
}

SerialLink::FrameType_t SerialLink::receiveFrameHead()
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

            return( None );
        }
    }
    char frameHead;
    if ( serial->read(&frameHead, 1) < 0 )
    {
        emit error( "Failed to receive frame type [serial error]",
                    serial->error() );
        return( None );
    }

    return( (FrameType_t)frameHead );
}

QByteArray SerialLink::receiveNextFrame()
{
    QByteArray result;
    FrameType_t type = receiveFrameHead();
    quint8 length = 0;
    switch( type )
    {
        case Response:
            length = respLength;
            break;
        case Data:
            length = dataFrameSize;
            break;
        case None:
        case Command:
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
