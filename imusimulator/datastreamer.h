#ifndef DATASTREAMER_H
#define DATASTREAMER_H

#include <QObject>
#include <QTimer>
#include "seriallinker.h"
#include "imudata.h"

class DataStreamer : public QObject
{
    Q_OBJECT
public:
    explicit DataStreamer(SerialLinker *link, IMUData *imu, QObject *parent = 0);
    ~DataStreamer();

private:
    SerialLinker *streamLink;
    IMUData      *imuData;
    QTimer       *samplingTimer;

    bool isRunning = false,
         sendingMode = false,
         timeToSend = false;
    qint32  spentTime = 0,
            timeStep_ms = 2;

    void initSamplingTimer();
    void setSendingFlag();
    void processSendingIMUData();

signals:
    void finished();
    void error( QString errStr, qint64 errCode );
    void sendConnectionState( bool );

public slots:
    void process();
    void stopStream();
};

#endif // DATASTREAMER_H
