#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QDebug>
#include <qbluetoothserver.h>
#include <qbluetoothserviceinfo.h>
#include <qbluetoothlocaldevice.h>
#include <QRandomGenerator>
#include <QMutex>

#include <pid_controller_model.h>
#include <scorp.h>
#include <plotmodel.h>


class Backend : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool showScanner READ showScanner NOTIFY showScannerChanged)
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:

    explicit Backend(QObject *parent = nullptr);

    bool showScanner() const;
    void setShowScanner(bool value);

    QString statusMessage() const;
    void setStatusMessage(QString value);

    Q_INVOKABLE void connectToService(QString address, QString uuid);
    Q_INVOKABLE void disconnectFromService();
    Q_INVOKABLE void connect2HardService();

    PIDControllerModel  m_ratesRollPitchModel;
    PIDControllerModel  m_ratesYawModel;

    PlotModel           m_rollModel;
    PlotModel           m_pitchModel;

Q_SIGNALS:
    void showScannerChanged();
    void statusMessageChanged();

public slots:
    void newDataReceived(SCORP::data_package_t &data);

    void btSocketError(QBluetoothSocket::SocketError error);
    void btSocketConnected();
    void btSocketDisconnected();
    void btSocketReadyRead();

    void sendPIDMainRates(QVector<float> &rates);
    void sendPIDYawRates(QVector<float> &rates);

    void receivePIDMainRates(QVector<float> &rates, SCORP::RatesType type);

    void ll_send_request(QByteArray &data);

    void toggleChartState(bool enabled);
    void updatePlotPeriod(quint32 periodMs);
    void updatePlotPointCount(QString pointCountText);
    void emulateDataReceive();

private:

    quint32             m_stepRangeX_ms;        // Plot period

    SCORP               m_protocol;

    bool                m_showScanner;
    QString             m_statusMessage;

    QBluetoothUuid      m_service_uuid;
    QBluetoothAddress   m_device_address;
    QBluetoothSocket    *m_btSocket;

    QMutex              m_socketMutex;

    bool                m_debugMode;

    QTimer              m_dataSimulatorTimer;


    void clearBTInput();
    void resetConnection();
    void generatePlotData();

};

#endif // BACKEND_H
