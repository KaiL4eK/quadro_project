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
    SerialLink(QVector<double> *p_thrustData, QVector<double> *p_torqueData,
               QVector<double> *p_currentData, QVector<double> *p_SpeedData,
               QVector<double> *p_timeData , QString sName, QObject *parent = 0);
    ~SerialLink();

private:
    QVector<double> *thrustList = NULL,
                    *torqueList = NULL,
                    *currentList = NULL,
                    *speedList = NULL,
                    *timeList = NULL;

    qint64  receivedPoints = 0;
    qint32  serialSpeed = 460800;
    QString serialName;
    bool    receiveData = false,
            isRunning = false;

    QSerialPort *serial;

    void clearDataBase( void );
    void initSerial( void );

    quint8 receiveFrameHead();
    QByteArray receiveNextFrame();

    bool makeLinkToPort();
    bool processConnectCommand();
    bool processDisconnectCommand();
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
    void sendMotorStartStopFinished(bool completed);
    void error( QString, qint64 );

public slots:
    void process();
    void stopLink();
    void processStartStopMotorCommand(bool startFlag, quint8 speed);
};

#endif // SERIALLINK_H
