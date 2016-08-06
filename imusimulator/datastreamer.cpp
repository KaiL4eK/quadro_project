#include "datastreamer.h"
#include <QThread>
#include <QDebug>
#include <QCoreApplication>
#include <QDataStream>

DataStreamer::DataStreamer(SerialLinker *link, IMUData *imu, QObject *parent) :
    QObject(parent), streamLink( link ), imuData( imu ), isRunning( false )
{

}

DataStreamer::~DataStreamer()
{
    qDebug() << "Deleting stream";
    emit sendConnectionState( false );
    delete streamLink;
}

void DataStreamer::initSamplingTimer()
{
    samplingTimer = new QTimer();
    connect( samplingTimer, &QTimer::timeout, this, &DataStreamer::setSendingFlag );
    samplingTimer->start( timeStep_ms );
}

void DataStreamer::setSendingFlag()
{
    timeToSend = true;
}

void DataStreamer::processSendingIMUData()
{
    QByteArray sendBuffer;
    qint32 rollAngle = imuData->rollAngle1000,
           pitchAngle = imuData->pitchAngle1000,
           timeMoments = spentTime*10;
    QDataStream bufferStream( &sendBuffer, QIODevice::ReadWrite );
    bufferStream << rollAngle << pitchAngle << timeMoments;
    spentTime += timeStep_ms;

    streamLink->sendIMUData( sendBuffer );
    qDebug() << "Sending data";
}

void DataStreamer::process()
{
    if ( streamLink->makeLinkToPort() )
    {
        emit sendConnectionState( true );
        isRunning = true;
        initSamplingTimer();
    }

    while ( isRunning )
    {
        switch ( streamLink->receiveCommand() )
        {
            case SerialLinker::NoCommand:
                break;
            case SerialLinker::Connect:
                if ( !streamLink->replyConnectCommand() )
                    isRunning = false;
                break;
            case SerialLinker::DataStart:
                if ( !streamLink->replyConnectCommand() )
                    isRunning = false;
                else
                {
                    sendingMode = true;
                    spentTime = 0;
                }
                break;
            case SerialLinker::DataStop:
                if ( !streamLink->replyConnectCommand() )
                    isRunning = false;
                sendingMode = false;
                break;
        }

        if ( sendingMode && timeToSend )
        {
            processSendingIMUData();
            timeToSend = false;
        }

        QCoreApplication::processEvents();
    }
    emit finished();
}

void DataStreamer::stopStream()
{
    isRunning = false;
}
