#include "mainwindow.h"
#include "serialLink.h"
#include <QObject>
#include <QPixmap>
#include <QLabel>
#include <QDebug>
#include <QMessageBox>
#include <QLineEdit>

#include <qwt_plot.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    connectionBtn = new QPushButton(tr("Connect"));
//    connectionBtn->setMinimumHeight(55);
    connectionBtn->setCheckable(true);

    motorControlBtn = new QPushButton("Motor start");
    motorControlBtn->setCheckable(true);
    serialNameFld = new QLineEdit();
    serialNameFld->setText(tr("/dev/ttyUSB0"));
    motorSpeedFld = new QLineEdit();
    motorSpeedFld->setText("0");

    QPixmap LETIcrestImage("crest.png");
    QPushButton *aboutBtn = new QPushButton();
    QIcon ButtonIcon(LETIcrestImage);
    aboutBtn->setIcon(ButtonIcon);
    aboutBtn->setIconSize(QSize(80, 80));

    QGridLayout *controlLayout = new QGridLayout();
    controlLayout->addWidget(connectionBtn, 0, 0);
    controlLayout->addWidget(serialNameFld, 0, 1);

    controlLayout->addWidget(motorControlBtn, 2, 0);
    controlLayout->addWidget(motorSpeedFld, 2, 1, 1, 2);
    controlLayout->addWidget(aboutBtn, 0, 3, 3, 1, Qt::AlignBottom|Qt::AlignRight);

    controlWidget = new QWidget(this);
    controlWidget->setLayout(controlLayout);

    QPushButton *hideBtn = new QPushButton();
    hideBtn->setCheckable(true);
    hideBtn->setMaximumHeight(20);

    QwtPlot *thrustPlot = new QwtPlot(),
            *torquePlot = new QwtPlot(),
            *currentPlot = new QwtPlot(),
            *speedPlot = new QwtPlot();

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(thrustPlot, 0, 0);
    layout->addWidget(torquePlot, 1, 0);
    layout->addWidget(currentPlot, 2, 0);
    layout->addWidget(speedPlot, 3, 0);
    layout->addWidget(hideBtn, 4, 0);
    layout->addWidget(controlWidget, 5, 0);

    QWidget *window = new QWidget();
    window->setLayout(layout);
    window->setMinimumHeight(600);
    window->setMinimumWidth(400);
    setCentralWidget(window);
    setWindowTitle(tr("3D Stand"));

    thrustPlotter =  new QwtPlotter( thrustPlot, &thrustData, &timeData, "Thrust" );
    torquePlotter = new QwtPlotter( torquePlot, &torqueData, &timeData, "Torque" );
    speedPlotter =  new QwtPlotter( speedPlot, &speedData, &timeData, "Speed" );
    currentPlotter = new QwtPlotter( currentPlot, &currentData, &timeData, "Current" );

    setDisconnectedUIState();

    connect( connectionBtn,    &QPushButton::clicked, this, &MainWindow::onConnectionBtnClick );
    connect( motorControlBtn,  &QPushButton::clicked, this, &MainWindow::onMotorStartBtnClick );
    connect( aboutBtn,         &QPushButton::clicked, this, &MainWindow::onAboutBtnClicked );
    connect( hideBtn,          &QPushButton::clicked, this, &MainWindow::onHideBtnClicked );


}

MainWindow::~MainWindow()
{
    delete connectionBtn;
    delete serialNameFld;
}

void MainWindow::createNewLink()
{
    QThread *linkThread = new QThread;
    SerialLink *serial = new SerialLink( &thrustData, &torqueData, &currentData, &speedData, &timeData, serialNameFld->text() );
    serial->moveToThread( linkThread );

    connect( serial, SIGNAL(error(QString, qint64)), this, SLOT(errorHandler(QString, qint64)) );

    connect( serial, &SerialLink::dataReceived, thrustPlotter, &QwtPlotter::dataProcess );
    connect( serial, &SerialLink::dataReceived, torquePlotter, &QwtPlotter::dataProcess );
    connect( serial, &SerialLink::dataReceived, speedPlotter, &QwtPlotter::dataProcess );
    connect( serial, &SerialLink::dataReceived, currentPlotter, &QwtPlotter::dataProcess );

    connect( linkThread, SIGNAL(started()), serial, SLOT(process()) );

    connect( this, &MainWindow::stopDataLink, serial, &SerialLink::stopLink );

    connect( serial, SIGNAL(finished()), linkThread, SLOT(quit()) );
    connect( serial, &SerialLink::finished, serial, &SerialLink::deleteLater );
    connect( linkThread, SIGNAL(finished()), linkThread, SLOT(deleteLater()) );
    connect( serial, &SerialLink::sendConnectionState, this, &MainWindow::changeConnectionState );

    connect( this, &MainWindow::sendStartStopMotorSignal, serial, &SerialLink::processStartStopMotorCommand );
    connect( serial, &SerialLink::sendMotorStartStopFinished, this, &MainWindow::motorStartStopReady );

    linkThread->start();
}

void MainWindow::setConnectedUIState()
{
    connectionBtn->setChecked(true);
    serialNameFld->setEnabled(false);
    motorControlBtn->setEnabled(true);
    motorSpeedFld->setEnabled(true);
}

void MainWindow::setDisconnectedUIState()
{
    connectionBtn->setChecked(false);
    serialNameFld->setEnabled(true);
    motorControlBtn->setEnabled(false);
    motorSpeedFld->setEnabled(false);
    motorControlBtn->setChecked(false);
}

void MainWindow::onHideBtnClicked(bool state)
{
    controlWidget->setVisible(!state);
}

void MainWindow::onAboutBtnClicked()
{
    QMessageBox::about(this, "About",
                             "3D Stand plot tool\n"
                             "Made in ACS Department\n\n"
                             "Please, all bugs info to avdevyatkin@etu.ru\n\n"
                             "2016");
}

void MainWindow::onMotorStartBtnClick(bool state)
{
    int power = motorSpeedFld->text().toShort();
    if ( power >= 0 && power <= 100 )
    {
        quint8  powerSend = power;
        emit sendStartStopMotorSignal(state, powerSend);
        if ( state )
        {
            motorSpeedFld->setEnabled(false);
        }
        else
        {
            motorSpeedFld->setEnabled(true);
        }
    }
    else
        QMessageBox::critical(this, "Power value",
                              "Error: Input correct power number");
}

void MainWindow::motorStartStopReady(bool completed)
{

}

void MainWindow::changeConnectionState( bool state )
{
    if ( state ) // Now connected
    {
        connectionBtn->setText( "Disconnect" );
        setConnectedUIState();
    }
    else // Now disconnected
    {
        connectionBtn->setText( "Connect" );
        setDisconnectedUIState();
    }

    connectionBtn->setEnabled( true );
}

void MainWindow::onConnectionBtnClick(bool state)
{
    connectionBtn->setEnabled( false );
    if ( state )
    {   // Was Disconnected
        createNewLink();
    }
    else
    {   // Was Connected
        emit stopDataLink();
    }
}

void MainWindow::errorHandler(QString errMsg, qint64 errCode)
{
    QMessageBox::critical(this, "Error " + QString::number(errCode),
                          errMsg);
}
