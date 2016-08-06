#include "seriallinker.h"

SerialLinker::SerialLinker(QString sName, qint32 sSpeed, QObject *parent) :
    QObject( parent ), serialName( sName ), serialSpeed( sSpeed )
{

}

SerialLinker::~SerialLinker()
{
    serial->readAll();
    serial->close();
    delete serial;
}

bool SerialLinker::makeLinkToPort()
{
    initLink();
    if ( serial->open( QIODevice::ReadWrite ) )
    {
        return( true );
    }
    emit error( "Failed to connect to serial port: " + serial->portName(), serial->error() );
    return( false );
}

SerialLinker::LinkCommand SerialLinker::receiveCommand()
{
    QString commandString;
    LinkCommand command = NoCommand;
    if ( serial->bytesAvailable() >= commandLength )
    {
        commandString = QString( serial->read( commandLength ) );
        if ( commandString.length() == commandLength )
        {
            if ( commandString == "con" )
                command = Connect;
            if ( commandString == "dst" )
                command = DataStart;
            if ( commandString == "psd" )
                command = DataStop;

            qDebug() << "Command: " << commandString;
        }
    }

    return( command );
}

bool SerialLinker::replyConnectCommand()
{
    if ( serial->write( QString("ok!").toUtf8().left( commandLength ) ) < 0 )
    {
        emit error( "Failed to write responce to serial port: " + serial->portName(), serial->error() );
        return( false );
    }
    return( true );
}

bool SerialLinker::sendIMUData(QByteArray &buffer)
{
    if ( serial->write( buffer ) < 0 )
    {
        emit error( "Failed to write responce to serial port: " + serial->portName(), serial->error() );
        return( false );
    }
    return( true );
}



int SerialLinker::initLink()
{
    serial = new QSerialPort( serialName );
    serial->setBaudRate( serialSpeed );
    serial->setDataBits( QSerialPort::Data8 );
    serial->setParity( QSerialPort::NoParity );
    serial->setStopBits( QSerialPort::OneStop );
    serial->setFlowControl( QSerialPort::NoFlowControl );
    return( 0 );
}
