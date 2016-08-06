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

void SerialLink::parseDataFrame(QByteArray &frame)
{
    qint16  rollAngle,
            pitchAngle,
            timeMoment,
            encoderRoll,
            encoderPitch;

    QByteArray tmpBuffer = frame.mid(0, 2);
    QDataStream streamRoll( &tmpBuffer, QIODevice::ReadWrite );
    streamRoll >> rollAngle;
    tmpBuffer = frame.mid(2, 2);
    QDataStream streamPitch( &tmpBuffer, QIODevice::ReadWrite );
    streamPitch >> pitchAngle;
    tmpBuffer = frame.mid(4, 2);
    QDataStream streamTime( &tmpBuffer, QIODevice::ReadWrite );
    streamTime >> timeMoment;
    tmpBuffer = frame.mid(6, 2);
    QDataStream streamEncR( &tmpBuffer, QIODevice::ReadWrite );
    streamEncR >> encoderRoll;
    tmpBuffer = frame.mid(8, 2);
    QDataStream streamEncP( &tmpBuffer, QIODevice::ReadWrite );
    streamEncP >> encoderPitch;

    qDebug() << rollAngle/250.0f << "\t" << pitchAngle/250.0f << "\t"
             << timeMoment*10.0f << "\t" << encoderRoll/250.0f << "\t"
             << encoderPitch/250.0f;

    rollDataList->push_back( rollAngle/250.0f );
    pitchDataList->push_back( pitchAngle/250.0f );
    encRollDataList->push_back( encoderRoll/250.0f + encoderRollCOffset );
    encPitchDataList->push_back( encoderPitch/250.0f + encoderPitchCOffset );
    timeList->push_back( timeMoment*10.0f ); // milliseconds

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
