#ifndef SCORP_H
#define SCORP_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <QIODevice>

/* Serial Connection Overall Reliable Protocol */

class SCORP : public QObject
{
    Q_OBJECT

public:

    enum RatesType {
        RATES_TYPE_PID_ROLL_PITCH,
        RATES_TYPE_PID_YAW
    };

    explicit SCORP(QObject *parent = nullptr);
    void connectSlave();
    void disconnectSlave();

    void sendRates(QVector<float> &rates, RatesType type);

    void stopDataSending();
    void startDataSending();


    void processInputData( QByteArray &data );



    /********** COMMON CONFIGURATION **********/

    #define FRAME_START         0x7E

    #define COMMAND_CONNECT         (1 << 0)
    #define COMMAND_DATA_START      (1 << 1)
    #define COMMAND_DATA_STOP       (1 << 2)
    #define COMMAND_DATA_PACK       (1 << 3)
    #define COMMAND_PING            (1 << 4)
    #define COMMAND_SET_MAIN_RATES  (1 << 5)
    #define COMMAND_SET_YAW_RATES   (1 << 6)

    typedef uint8_t frame_command_t;

    typedef struct {
        float       rates[3];
    } pid_rates_t;

    typedef struct {
        /* [ rp[P, I, D], y[P, I, D] ] */
        pid_rates_t PIDRates[2];
        uint16_t    plot_period_ms;

    } connect_response_t;

    typedef struct {
        bool            succeed;
    } response_ack_t;

    #define DATA_FLOAT_MULTIPLIER   100

    typedef struct {
        uint32_t        packId;

        int16_t         roll;
        int16_t         pitch;
        int16_t         yaw;

        int16_t         ref_roll;
        int16_t         ref_pitch;
        int16_t         ref_yaw;
    } data_package_t;

    uint8_t calcChksum( uint8_t *data, uint8_t len )
    {
        uint16_t data_byte;
        uint16_t crc = 0;

        while ( len-- )
        {
            data_byte = *data++;

            crc = (crc << 2) + crc + data_byte;
            crc = (crc << 2) + crc + data_byte;
            crc = crc ^ (crc >> 8);
        }

        return (crc & 0xFF);
    }

    typedef enum {
        WAIT_4_START,
        WAIT_4_COMMAND,
        WAIT_4_DATA,
        WAIT_4_CKSUM

    } receive_mode_t;

    /********** COMMON CONFIGURATION END **********/

public slots:

    void responseTimeout();
    void dataTransferPingTimeout();
Q_SIGNALS:
    void sendData( QByteArray &data );
    void resetInterfaceConnection();

    void updateRates(QVector<float> &rates, SCORP::RatesType type);
    void newDataReceived(SCORP::data_package_t &data);
    void updatePlotPeriod(quint32 periodMs);

private:
    bool                m_isConnected;

    bool sendConnectCommand();
    bool sendPingCommand();
    bool sendPIDRatesCommand(frame_command_t cmd, pid_rates_t *rates);
    bool sendDataCommand(bool enable);

    QTimer              m_responseTimer;
    QTimer              m_pingTimer;
    frame_command_t     m_cmdWaitResponse;

    receive_mode_t      m_waitState;

    QByteArray          m_inputBuffer;
    qint64              m_inputLength;
    frame_command_t     m_inputCmd;
    bool                m_isCksumReq;


};

#endif // SCORP_H
