#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    mainWgt         = new QWidget;
    mainLayout      = new QVBoxLayout;

    inputWgt        = new QWidget;
    inputLayout     = new QGridLayout;

    controlLayout   = new QGridLayout;
    controlWgt      = new QWidget;

    m_cData         = new ControlData;
    deviceSelect    = new DeviceBTSelect;

    ControlRates    rates = m_cData->getControlRates();

    mainWgt->setLayout( mainLayout );

    setCentralWidget( mainWgt );
    resize( 320, 240 );

    mainLayout->addWidget( controlWgt, 0 );
    mainLayout->addWidget( inputWgt, 1 );

    QDoubleValidator    *validator = new QDoubleValidator;
    p_pos_rate_fld = new QLineEdit( QString::number(rates.p_pos) );
    p_pos_rate_fld->setValidator( validator );
    i_pos_rate_fld = new QLineEdit( QString::number(rates.i_pos) );
    i_pos_rate_fld->setValidator( validator );
    p_spd_rate_fld = new QLineEdit( QString::number(rates.p_spd) );
    p_spd_rate_fld->setValidator( validator );
    d_spd_rate_fld = new QLineEdit( QString::number(rates.d_spd) );
    d_spd_rate_fld->setValidator( validator );

    p_pos_rate_set_btn  = new QPushButton( "Set P pos" );
    i_pos_rate_set_btn  = new QPushButton( "Set I pos" );
    p_spd_rate_set_btn  = new QPushButton( "Set P spd" );
    d_spd_rate_set_btn  = new QPushButton( "Set D spd" );

    inputWgt->setLayout( inputLayout );

    inputLayout->addWidget( p_pos_rate_fld, 0, 0, 1, 2 );
    inputLayout->addWidget( i_pos_rate_fld, 1, 0, 1, 2 );
    inputLayout->addWidget( p_spd_rate_fld, 2, 0, 1, 2 );
    inputLayout->addWidget( d_spd_rate_fld, 3, 0, 1, 2 );

    inputLayout->addWidget( p_pos_rate_set_btn, 0, 2 );
    inputLayout->addWidget( i_pos_rate_set_btn, 1, 2 );
    inputLayout->addWidget( p_spd_rate_set_btn, 2, 2 );
    inputLayout->addWidget( d_spd_rate_set_btn, 3, 2 );

    connect( p_pos_rate_fld, SIGNAL(editingFinished()), this, SLOT(rate_flds_edit_fin()) );
    connect( i_pos_rate_fld, SIGNAL(editingFinished()), this, SLOT(rate_flds_edit_fin()) );
    connect( p_spd_rate_fld, SIGNAL(editingFinished()), this, SLOT(rate_flds_edit_fin()) );
    connect( d_spd_rate_fld, SIGNAL(editingFinished()), this, SLOT(rate_flds_edit_fin()) );

    deviceConnectBtn    = new QPushButton( "Connect" );
    deviceConnectBtn->setCheckable( true );

    controlWgt->setLayout( controlLayout );

    controlLayout->addWidget( deviceConnectBtn );

    connect( deviceConnectBtn, SIGNAL(clicked(bool)), this, SLOT(connectBtnPressed(bool)) );
}

MainWindow::~MainWindow()
{
    delete p_pos_rate_fld;
    delete i_pos_rate_fld;
    delete p_spd_rate_fld;
    delete d_spd_rate_fld;

    delete p_pos_rate_set_btn;
    delete i_pos_rate_set_btn;
    delete p_spd_rate_set_btn;
    delete d_spd_rate_set_btn;


}


void MainWindow::p_pos_rate_btn_clicked ( bool clicked )
{
    clicked = clicked;

    qDebug() << "Clicked";
}

void MainWindow::rate_flds_edit_fin ( void )
{
    ControlRates    rates;

    qDebug() << "Rate fin";

    rates.p_pos = p_pos_rate_fld->text().toFloat();
    rates.i_pos = i_pos_rate_fld->text().toFloat();
    rates.p_spd = p_spd_rate_fld->text().toFloat();
    rates.d_spd = d_spd_rate_fld->text().toFloat();

    m_cData->updateControlRates( rates );
}

void MainWindow::connectBtnPressed ( bool clicked )
{
    if ( clicked )
    {
        if ( !DeviceBTSelect::checkBTEnabled( this ) )
        {
            deviceConnectBtn->setChecked( false );
            return;
        }

        if ( !deviceSelect->connectDevice() )
        {
            deviceConnectBtn->setChecked( false );
            return;
        }

        deviceConnectBtn->setText( "Disconnect" );
    }
    else
    {
        // Disconnect
        deviceSelect->disconnectDevice();
        deviceConnectBtn->setText( "Connect" );
    }
}
