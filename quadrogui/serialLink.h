#ifndef SERIALLINK_H
#define SERIALLINK_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QtSerialPort/QSerialPort>
#include <QDataStream>

class SerialLink : public QObject
{
    Q_OBJECT

public:
    SerialLink(QVector<double> *rollDbPtr, QVector<double> *pitchDbPtr,
               QVector<double> *encRollDbPtr, QVector<double> *encPitchDbPtr,
               QVector<double> *timeVectPtr , QString sName, QObject *parent = 0);
    ~SerialLink();

private:
    QVector<double> *rollDataList = NULL,
                    *pitchDataList = NULL,
                    *encRollDataList = NULL,
                    *encPitchDataList = NULL,
                    *timeList = NULL;

    enum FrameType_t
    {
        Command = '*',
        Parameter = '~',
        Response = '#',
        Data = '$',
        None = 0
    };

    qint64  receivedPoints = 0;
    qint32  serialSpeed = 460800;
    QString serialName;
    bool    receiveData = false,
            isRunning = false;

    QSerialPort *serial;

    double  encoderRollCOffset = 0.0,
            encoderPitchCOffset = 0.0;
    quint8  calibrationCounter = 0;
    bool    calibrationFlag = false;

    const char  cmdConnect =        'c',
                cmdDataStart =      's',
                cmdDataStop =       'p',
                paramMotorStart =   's',
                paramMotorStop =    't';

    const int   commandLength = 2,
                respLength =    1,
                dataFrameSize = 10;

    const char  noErrorResponse =   '0',
                dataStopResp =      '2';

    void clearDataBase( void );
    void initSerial( void );

    int initLink();

    FrameType_t receiveFrameHead();
    QByteArray receiveNextFrame();

    bool makeLinkToPort();
    bool processConnectionCommand();
    bool processDataStartCommand();
    bool processDataStopCommand();
    bool processMotorStartCommand(quint8 speed);
    bool processMotorStopCommand();
    bool processReceiveData();
    bool waitForResponse();
    void parseDataFrame(QByteArray &frame);

signals:
    void dataReceived();
    void finished();
    void sendConnectionState(bool state);
    void sendCalibrationReady(bool completed);
    void sendMotorStartStopFinished(bool completed);
    void error( QString, qint64 );

public slots:
    void process();
    void stopLink();
    void setDataCommandFlag( bool dataRcvBtnState );
    void processStartStopMotorCommand(bool startFlag, quint8 speed);
    void calibrateSources();
};

#endif // SERIALLINK_H
