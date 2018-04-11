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

/************************************************************/



ControlDataView::ControlDataView( QString name ):
    m_name( name )
{
    ControlRates    rates = m_cData.getControlRates();

    QDoubleValidator    *validator = new QDoubleValidator;
    p_rate_fld = new QLineEdit( QString::number(rates.p_rate) );
    p_rate_fld->setValidator( validator );
    i_rate_fld = new QLineEdit( QString::number(rates.i_rate) );
    i_rate_fld->setValidator( validator );
    d_rate_fld = new QLineEdit( QString::number(rates.d_rate) );
    d_rate_fld->setValidator( validator );

    wgt                     = new QWidget;

    QVBoxLayout *layout     = new QVBoxLayout;

    wgt->setLayout( layout );

    layout->addWidget( new QLabel( "Rates " + m_name ) );

    layout->addWidget( p_rate_fld );
    layout->addWidget( i_rate_fld );
    layout->addWidget( d_rate_fld );

    connect( p_rate_fld, SIGNAL(editingFinished()), this, SLOT(rateFldsEditFin()) );
    connect( i_rate_fld, SIGNAL(editingFinished()), this, SLOT(rateFldsEditFin()) );
    connect( d_rate_fld, SIGNAL(editingFinished()), this, SLOT(rateFldsEditFin()) );
}

void ControlDataView::rateFldsEditFin ( void )
{
    ControlRates    rates;

    qDebug() << "Rate fin";

    rates.p_rate = p_rate_fld->text().toFloat();
    rates.i_rate = i_rate_fld->text().toFloat();
    rates.d_rate = d_rate_fld->text().toFloat();

    m_cData.updateControlRates( rates );
}
