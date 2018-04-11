#ifndef CONTROLDATA_H
#define CONTROLDATA_H

#include <QDebug>
#include <QMainWindow>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include <QDoubleValidator>

struct ControlRates
{
    float p_rate;
    float i_rate;
    float d_rate;

    ControlRates() :
        p_rate( 0 ), i_rate( 0 ), d_rate( 0 )
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

class ControlDataView: public QObject
{
    Q_OBJECT

    ControlData m_cData;

public:
    ControlDataView( QString name );

    QWidget *widget() { return wgt; }

private slots:
    void rateFldsEditFin();

private:
    QString             m_name;

    QLineEdit           *p_rate_fld;
    QLineEdit           *i_rate_fld;
    QLineEdit           *d_rate_fld;

    QWidget             *wgt;
};

#endif // CONTROLDATA_H
