#ifndef CONTROLDATA_H
#define CONTROLDATA_H

#include <QDebug>

struct ControlRates
{
    float p_pos;
    float i_pos;
    float p_spd;
    float d_spd;

    ControlRates() :
        p_pos( 0 ), i_pos( 0 ),
        p_spd( 0 ), d_spd( 0 )
    {
    }

};

class ControlData
{
public:
    ControlData();

    void updateControlRates(ControlRates &rates);
    ControlRates & getControlRates( void );

private:
    ControlRates    m_rates;

};

#endif // CONTROLDATA_H
