#ifndef SERIALLINK_H
#define SERIALLINK_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QtSerialPort/QSerialPort>
#include <QDataStream>

enum DataIndex
{
    ThrustDataIndex     = 0,
    TorqueDataIndex     = 1,
    CurrentDataIndex    = 2,
    SpeedDataIndex      = 3
};

struct SerialFrame
{
    uint8_t     type;
    uint8_t     length;
    QByteArray  data;

    void clear() { type = 0; data.clear(); }
};

class SerialLink : public QObject
{
    Q_OBJECT

public:
    SerialLink(QString sName, QObject *parent = 0);
    ~SerialLink();

private:
    QVector<QVector<double>>    ma_thrustData,
                                ma_torqueData,
                                ma_currentData,
                                ma_speedData,
                                ma_timeData;

    uint16_t    current_plot    = 0;
    uint32_t    serialSpeed     = 460800;

    QString     serialName;
    bool        receiveData     = false,
                isRunning       = false;

    QSerialPort *serial;
    SerialFrame frame;

    void clearDataBase( void );
    void initSerial( void );

    void receiveFrameHeader();
    void receiveNextFrame();

    bool makeLinkToPort();
    bool processConnectCommand();
    bool processDisconnectCommand();
    bool processDataStartCommand();
    bool processDataStopCommand();
    bool processMotorStartCommand(quint8 speed);
    bool processMotorStopCommand();
    bool processReceiveData();
    bool receiveResponse();
    void parseDataFrame(QByteArray &frame);

signals:
    void dataReceived();
    void finished();
    void sendConnectionState(bool state);
    void sendMotorStartStopFinished(bool completed);
    void error( QString, qint64 );
    void setDataSource(uint16_t plotId, QVector<QVector<double>> *data_vect, QVector<QVector<double>> *time_vect);

public slots:
    void process();
    void stopLink();
    void processStartStopMotorCommand(bool startFlag, quint8 speed);
};

#endif // SERIALLINK_H
