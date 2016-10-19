#ifndef SERIALLINK_H
#define SERIALLINK_H

#include <QObject>
#include <QThread>
#include <QVector>
#include <QtSerialPort/QSerialPort>
#include <QDataStream>

#include <serial_protocol.h>

struct MeasureParams
{
    uint8_t     motor_power_start;
    uint8_t     motor_power_stop;
    uint16_t    time_measure_start_ms;
    uint16_t    time_measure_ms;
    uint16_t    time_step_moment_ms;
};

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

    QString     serialName;
    bool        receiveData     = false,
                isRunning       = false,
                parametersSent  = false;

    qint16      current_plot;
    uint32_t    serialSpeed;

    QSerialPort *serial;
    SerialFrame frame;

    void resizeDatabase( void );
    void initSerial( void );

    void receiveFrameHeader();
    void receiveNextFrame();

    bool makeLinkToPort();
    bool processConnectCommand();
    bool processDisconnectCommand();
    bool processDataStartCommand();
    bool processDataStopCommand();
    bool processMeasureStartCommand();
    bool processMeasureStopCommand();
    bool processReceiveData();
    bool receiveResponse();
    void parseDataFrame(QByteArray &frame);

signals:
    void dataReceived();
    void finished();
    void sendConnectionState(bool state);
    void sendDataReceiveFinished();
    void error( QString, qint64 );
    void addNewCurve(quint16 plotId);
    void setDataSource(quint16 plotId, QVector<QVector<double>> *data_vect, QVector<QVector<double>> *time_vect);

public slots:
    void process();
    void stopLink();
    void processStartStopMotorCommand(bool startFlag);
    void processSetParametersCommand(MeasureParams);
};

#endif // SERIALLINK_H
