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
    SerialLink(QString sName, QObject *parent = 0);
    ~SerialLink();

private:
    QVector<QVector<double>>    thrustList,
                                torqueList,
                                currentList,
                                speedList,
                                timeList;

    quint16 current_plot    = 0;
    qint32  serialSpeed     = 460800;
    QString serialName;
    bool    receiveData     = false,
            isRunning       = false;

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
