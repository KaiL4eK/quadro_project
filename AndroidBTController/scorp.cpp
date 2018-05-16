#include "scorp.h"

SCORP::SCORP(QObject *parent) :
    QObject(parent), m_isConnected(false), m_waitState(WAIT_4_START)
{
    m_inputBuffer.clear();

    /* Timers */
    m_responseTimer.setSingleShot( true );
    m_responseTimer.setInterval( 5000 );
    connect(&m_responseTimer, SIGNAL(timeout()), this, SLOT(responseTimeout()));

    m_pingTimer.setInterval( 3000 );
    connect(&m_pingTimer, SIGNAL(timeout()), this, SLOT(dataTransferPingTimeout()));
}

void SCORP::responseTimeout()
{
    qDebug() << "Timeout!!!";
}

void SCORP::dataTransferPingTimeout()
{
    if ( !m_isConnected )
        return;

    qDebug() << "Data ping";
    sendPingCommand();
}

void SCORP::sendRates(QVector<float> &rates, RatesType type)
{
    if ( !m_isConnected )
        return;

    pid_rates_t send_rates = { rates[0], rates[1], rates[2] };

    qDebug() << "Send main rates";

    switch ( type )
    {
        case RATES_TYPE_PID_ROLL_PITCH:
            sendPIDRatesCommand( COMMAND_SET_MAIN_RATES, &send_rates );
            break;

        case RATES_TYPE_PID_YAW:
            sendPIDRatesCommand( COMMAND_SET_YAW_RATES, &send_rates );
            break;
    }
}

void SCORP::connectSlave()
{
    m_isConnected = true;
    m_waitState = WAIT_4_START;
    m_cmdWaitResponse = (frame_command_t)0;

    sendConnectCommand();
}

void SCORP::disconnectSlave()
{
    m_isConnected = false;
}

void SCORP::startDataSending()
{
    if ( !m_isConnected )
        return;

    sendDataCommand( true );
}

void SCORP::stopDataSending()
{
    if ( !m_isConnected )
        return;

    sendDataCommand( false );
}


/*** Commands ***/


bool SCORP::sendPingCommand()
{
    frame_command_t cmd = COMMAND_PING;

    if ( m_cmdWaitResponse & cmd )
    {
        qDebug() << "Command still pending";
        return false;
    }

    QByteArray  array;

    array += FRAME_START;
    array += cmd;

    m_cmdWaitResponse = (frame_command_t)(m_cmdWaitResponse | cmd);

    m_responseTimer.start();
    emit sendData( array );

    return true;
}

bool SCORP::sendDataCommand( bool enable )
{
    frame_command_t cmd = enable ? COMMAND_DATA_START : COMMAND_DATA_STOP;

    if ( m_cmdWaitResponse & cmd )
    {
        qDebug() << "Command still pending";
        return false;
    }

    if ( enable )
    {
        m_pingTimer.start();
    }
    else
    {
        m_pingTimer.stop();
        m_cmdWaitResponse = (frame_command_t)(m_cmdWaitResponse & ~COMMAND_DATA_PACK);
    }

    QByteArray  array;

    array += FRAME_START;
    array += cmd;

    m_cmdWaitResponse = (frame_command_t)(m_cmdWaitResponse | cmd);

//    qDebug() << "Called: " << m_cmdWaitResponse;

    m_responseTimer.start();
    emit sendData( array );

    return true;
}

bool SCORP::sendConnectCommand()
{
    frame_command_t cmd = COMMAND_CONNECT;

    if ( m_cmdWaitResponse & cmd )
    {
        qDebug() << "Command still pending";
        return false;
    }

    QByteArray  array;

    array += FRAME_START;
    array += cmd;

    m_cmdWaitResponse = (frame_command_t)(m_cmdWaitResponse | cmd);

    m_responseTimer.start();
    emit sendData( array );

    return true;
}

bool SCORP::sendPIDRatesCommand( frame_command_t cmd, pid_rates_t *rates )
{
    if ( m_cmdWaitResponse & cmd )
    {
        qDebug() << "Command still pending";
        return false;
    }

    QByteArray  array;

    array += FRAME_START;
    array += cmd;

    array += QByteArray( (char *)rates, sizeof( *rates ) );

    uint8_t calc_cksum = calcChksum( (uint8_t *)rates, sizeof( *rates ) );
    array += calc_cksum;

    m_cmdWaitResponse = (frame_command_t)(m_cmdWaitResponse | cmd);

    m_responseTimer.start();
    emit sendData( array );

    return true;
}

/*** Input processing ***/

void SCORP::processInputData( QByteArray &data )
{
    for( QByteArray::iterator it = data.begin(); it != data.end(); it++ )
    {
        bool    cmdIsExpected   = false;
        bool    isDataReceived  = false;
        quint8  in_byte         = (*it);

//        qDebug() << "Byte: " << QString::number( in_byte, 16 );

        quint8  calc_cksum      = 0;

        switch ( m_waitState )
        {
            case WAIT_4_START:
                if ( in_byte == FRAME_START )
                {
                    m_waitState = WAIT_4_COMMAND;
//                    qDebug() << "Frame started";
                }
                break;

            case WAIT_4_COMMAND:
                if ( in_byte & m_cmdWaitResponse )
                {
                    cmdIsExpected = true;
//                    qDebug() << "Command is expected: " << in_byte;
                }
                else
                {
                    m_waitState = WAIT_4_START;
                    qDebug() << "Command is not expected: " << in_byte << " / " << m_cmdWaitResponse;
                }
                break;

            case WAIT_4_DATA:
                m_inputBuffer += in_byte;

                if ( m_inputBuffer.length() >= m_inputLength )
                {
                    if ( m_isCksumReq )
                        m_waitState = WAIT_4_CKSUM;
                    else
                        isDataReceived = true;
                }
                break;

            case WAIT_4_CKSUM:
                calc_cksum = calcChksum( (uint8_t *)m_inputBuffer.data(), m_inputBuffer.length() );

                if ( calc_cksum == in_byte )
                {
                    isDataReceived = true;
                }
                else
                {
                    qDebug() << "Invalid cksum";
                    m_waitState     = WAIT_4_START;
                }

            default:
                ;
        }

        if ( cmdIsExpected )
        {
            switch ( in_byte )
            {
                case COMMAND_CONNECT:
                    m_isCksumReq    = true;
                    m_inputCmd      = (frame_command_t)in_byte;
                    m_inputLength   = sizeof( connect_response_t );
                    m_waitState     = WAIT_4_DATA;
                    m_inputBuffer.clear();
                    break;

                case COMMAND_DATA_PACK:
                    m_isCksumReq    = true;
                    m_inputCmd      = (frame_command_t)in_byte;
                    m_inputLength   = sizeof( data_package_t );
                    m_waitState     = WAIT_4_DATA;
                    m_inputBuffer.clear();
                    break;

                case COMMAND_PING:
                case COMMAND_DATA_START:
                case COMMAND_DATA_STOP:
                case COMMAND_SET_MAIN_RATES:
                case COMMAND_SET_YAW_RATES:
                    m_isCksumReq    = false;
                    m_inputCmd      = (frame_command_t)in_byte;
                    m_inputLength   = sizeof( response_ack_t );
                    m_waitState     = WAIT_4_DATA;
                    m_inputBuffer.clear();
                    break;

                default:
                    qDebug() << "Invalid command";
                    m_waitState     = WAIT_4_START;
            }
        }

        if ( isDataReceived )
        {
            /* Processing received data */

            m_waitState     = WAIT_4_START;
            /* Exclude command from waiting */


            switch ( m_inputCmd )
            {
                case COMMAND_CONNECT:
                    {
                        connect_response_t  *resp = (connect_response_t *)m_inputBuffer.data();

                        qDebug() << "Response:";
                        qDebug() << resp->PIDRates[0].rates[0] << " / " << resp->PIDRates[0].rates[1] << " / " << resp->PIDRates[0].rates[2];
                        qDebug() << resp->PIDRates[1].rates[0] << " / " << resp->PIDRates[1].rates[1] << " / " << resp->PIDRates[1].rates[2];

                        QVector<float>  rates;
                        rates.resize( 3 );

                        for (int i = 0; i < 3; i++)
                            rates[i] = resp->PIDRates[0].rates[i];

                        emit updateRates( rates, RATES_TYPE_PID_ROLL_PITCH );

                        for (int i = 0; i < 3; i++)
                            rates[i] = resp->PIDRates[1].rates[i];

                        emit updateRates( rates, RATES_TYPE_PID_YAW );

                        emit updatePlotPeriod( resp->plot_period_ms );

                        m_cmdWaitResponse = (frame_command_t)(m_cmdWaitResponse & ~m_inputCmd);

                        m_responseTimer.stop();
                    }
                    break;

                case COMMAND_DATA_PACK:
                    {
                        data_package_t  data_p = *(data_package_t *)m_inputBuffer.data();

                        emit newDataReceived( data_p );
                    }
                    break;

                case COMMAND_DATA_STOP:
                case COMMAND_SET_MAIN_RATES:
                case COMMAND_SET_YAW_RATES:
                case COMMAND_PING:
                case COMMAND_DATA_START:
                    {
                        response_ack_t  *resp =    (response_ack_t *)m_inputBuffer.data();

                        qDebug() << "Response: " << resp->succeed;

                        m_cmdWaitResponse = (frame_command_t)(m_cmdWaitResponse & ~m_inputCmd);
                        m_responseTimer.stop();

                        if ( resp->succeed )
                        {
                            if ( m_inputCmd == COMMAND_DATA_START )
                                m_cmdWaitResponse = (frame_command_t)(m_cmdWaitResponse | COMMAND_DATA_PACK);
                        }
                    }
                    break;

                default:
                    m_waitState     = WAIT_4_START;
            }
        }
    }
}

