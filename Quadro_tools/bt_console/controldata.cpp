#include "controldata.h"

ControlData::ControlData()
{

}

void ControlData::updateControlRates(ControlRates &rates)
{
    m_rates = rates;

    qDebug() << "Update!" << __FUNCTION__;
}

ControlRates &ControlData::getControlRates()
{
    return m_rates;
}
