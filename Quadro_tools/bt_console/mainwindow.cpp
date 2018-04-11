#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget         *mainWgt        = new QWidget;
    QVBoxLayout     *mainLayout     = new QVBoxLayout;

    QWidget         *ratesWgt       = new QWidget;
    QHBoxLayout     *ratesLayout    = new QHBoxLayout;

    QGridLayout     *controlLayout  = new QGridLayout;
    QWidget         *controlWgt     = new QWidget;


    deviceSelect    = new DeviceBTSelect;

    mainWgt->setLayout( mainLayout );

    setCentralWidget( mainWgt );
//    resize( 320, 240 );

    /* Control panel */
    mainLayout->addWidget( controlWgt );
    controlWgt->setLayout( controlLayout );

    deviceConnectBtn    = new QPushButton( "Connect" );
    deviceConnectBtn->setCheckable( true );
    controlLayout->addWidget( deviceConnectBtn );
    connect( deviceConnectBtn, SIGNAL(clicked(bool)), this, SLOT(connectBtnPressed(bool)) );

    /* Rates models */
    ratesRollPitch  = new ControlDataView( "RollPitch" );
    ratesYaw        = new ControlDataView( "Yaw" );

    mainLayout->addWidget( ratesWgt );
    ratesWgt->setLayout( ratesLayout );
    ratesLayout->addWidget( ratesRollPitch->widget() );
    ratesLayout->addWidget( ratesYaw->widget() );

    /* Send rates button */
    send_data_btn       = new QPushButton( "Send rates" );
    mainLayout->addWidget( send_data_btn );
    connect( send_data_btn, SIGNAL(clicked(bool)), this, SLOT(sendDataBtnClicked(bool)) );

}

MainWindow::~MainWindow()
{

}


void MainWindow::sendDataBtnClicked ( bool clicked )
{
    clicked = clicked;

    qDebug() << "Clicked";
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
