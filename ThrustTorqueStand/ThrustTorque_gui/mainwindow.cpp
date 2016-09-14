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
    connectionBtn->setMinimumHeight(55);
    connectionBtn->setCheckable(true);
    calibratioBtn = new QPushButton(tr("Calibrate"));
    connectionBtn->setCheckable(true);
    dataReceiveBtn = new QPushButton("Receive data");
    dataReceiveBtn->setCheckable(true);
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
    controlLayout->addWidget(connectionBtn, 0, 0, 2, 1);
    controlLayout->addWidget(serialNameFld, 0, 1, 1, 2);
    controlLayout->addWidget(dataReceiveBtn, 1, 2);
    controlLayout->addWidget(calibratioBtn, 1, 1);
    controlLayout->addWidget(motorControlBtn, 2, 0);
    controlLayout->addWidget(motorSpeedFld, 2, 1, 1, 2);\
    controlLayout->addWidget(aboutBtn, 0, 3, 3, 1, Qt::AlignBottom|Qt::AlignRight);

    controlWidget = new QWidget(this);
    controlWidget->setLayout(controlLayout);

    QPushButton *hideBtn = new QPushButton();
    hideBtn->setCheckable(true);
    hideBtn->setMaximumHeight(20);

    QwtPlot *rollPlot = new QwtPlot(),
            *pitchPlot = new QwtPlot();

    motor1_power = new QProgressBar();
    motor1_power->setOrientation( Qt::Vertical );
    motor1_power->setRange(0, 100);
    motor1_power->setTextVisible(true);

    motor2_power = new QProgressBar();
    motor2_power->setOrientation( Qt::Vertical );
    motor2_power->setRange(0, 100);
    motor2_power->setTextVisible(true);

    motor3_power = new QProgressBar();
    motor3_power->setOrientation( Qt::Vertical );
    motor3_power->setRange(0, 100);
    motor3_power->setTextVisible(true);

    motor4_power = new QProgressBar();
    motor4_power->setOrientation( Qt::Vertical );
    motor4_power->setRange(0, 100);
    motor4_power->setTextVisible(true);

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(rollPlot, 0, 0);
    layout->addWidget(pitchPlot, 1, 0);
    layout->addWidget(hideBtn, 2, 0);
    layout->addWidget(controlWidget, 3, 0);
    layout->addWidget(motor2_power, 0, 1);
    layout->addWidget(motor1_power, 0, 2);
    layout->addWidget(motor3_power, 1, 1);
    layout->addWidget(motor4_power, 1, 2);

    QWidget *window = new QWidget();
    window->setLayout(layout);
    window->setMinimumHeight(400);
    window->setMinimumWidth(400);
    setCentralWidget(window);
    setWindowTitle(tr("3D Stand"));

    rollPlotter =  new QwtPlotter( rollPlot, &rollDB, &encRollDB, &timeDB, "Roll" );
    pitchPlotter = new QwtPlotter( pitchPlot, &pitchDB, &encPitchDB, &timeDB, "Pitch" );

    setDisconnectedUIState();

    connect( connectionBtn,    &QPushButton::clicked, this, &MainWindow::onConnectionBtnClick );
    connect( calibratioBtn,    &QPushButton::clicked, this, &MainWindow::onCalibrationBtnClick );
    connect( dataReceiveBtn,   &QPushButton::clicked, this, &MainWindow::onDataReceiveBtnClick );
    connect( motorControlBtn,  &QPushButton::clicked, this, &MainWindow::onMotorStartBtnClick );
    connect( aboutBtn,         &QPushButton::clicked, this, &MainWindow::onAboutBtnClicked );
    connect( hideBtn,          &QPushButton::clicked, this, &MainWindow::onHideBtnClicked );
}

MainWindow::~MainWindow()
{
    delete connectionBtn;
    delete serialNameFld;
    delete dataReceiveBtn;
}

void MainWindow::createNewLink()
{
    QThread *linkThread = new QThread;
    SerialLink *serial = new SerialLink( &rollDB, &pitchDB, &encRollDB, &encPitchDB, &timeDB, serialNameFld->text() );
    serial->moveToThread( linkThread );

    connect( serial, SIGNAL(error(QString, qint64)), this, SLOT(errorHandler(QString, qint64)) );

    connect( serial, &SerialLink::dataReceived, rollPlotter, &QwtPlotter::dataProcess );
    connect( serial, &SerialLink::dataReceived, pitchPlotter, &QwtPlotter::dataProcess );

    connect( linkThread, SIGNAL(started()), serial, SLOT(process()) );

    connect( this, &MainWindow::stopDataLink, serial, &SerialLink::stopLink );

    connect( serial, SIGNAL(finished()), linkThread, SLOT(quit()) );
    connect( serial, &SerialLink::finished, serial, &SerialLink::deleteLater );
    connect( linkThread, SIGNAL(finished()), linkThread, SLOT(deleteLater()) );
    connect( serial, &SerialLink::sendConnectionState, this, &MainWindow::changeConnectionState );

    connect( this, &MainWindow::callCalibration, serial, &SerialLink::calibrateSources );
    connect( serial, &SerialLink::sendCalibrationReady, this, &MainWindow::calibrationReady );

    connect( this, &MainWindow::sendDataProcessingSignal, serial, &SerialLink::setDataCommandFlag );

    connect( this, &MainWindow::sendStartStopMotorSignal, serial, &SerialLink::processStartStopMotorCommand );
    connect( serial, &SerialLink::sendMotorStartStopFinished, this, &MainWindow::motorStartStopReady );
    connect( serial, &SerialLink::sendMotorPowers, this, &MainWindow::updateMotorsPower );

    linkThread->start();
}

void MainWindow::setConnectedUIState()
{
    connectionBtn->setChecked(true);
    dataReceiveBtn->setEnabled(true);
    serialNameFld->setEnabled(false);
    calibratioBtn->setEnabled(true);
    motorControlBtn->setEnabled(true);
    motorSpeedFld->setEnabled(true);
}

void MainWindow::setDisconnectedUIState()
{
    connectionBtn->setChecked(false);
    dataReceiveBtn->setEnabled(false);
    dataReceiveBtn->setChecked(false);
    serialNameFld->setEnabled(true);
    calibratioBtn->setEnabled(false);
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

void MainWindow::onDataReceiveBtnClick(bool state)
{
    rollPlotter->refreshPlotView();
    pitchPlotter->refreshPlotView();
    emit sendDataProcessingSignal(state);
    if ( state )
    {
        calibratioBtn->setEnabled(false);
    }
    else
    {
        calibratioBtn->setEnabled(true);
    }
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

void MainWindow::onCalibrationBtnClick()
{
    calibratioBtn->setEnabled(false);
    calibratioBtn->setText("Calibrating...");
    dataReceiveBtn->setEnabled(false);
    QMessageBox::information(this, "Calibration",
                             "Set stand to calm position and\n"
                             "DON`T TOUCH!!! =)");
    emit callCalibration();
}

void MainWindow::calibrationReady(bool completed)
{
    calibratioBtn->setEnabled(true);
    calibratioBtn->setText("Calibrate");
    dataReceiveBtn->setEnabled(true);
    if ( completed )
        QMessageBox::information(this, "Calibration",
                                 "Completed");
    else
        QMessageBox::information(this, "Calibration",
                                 "Failed");
}

void MainWindow::errorHandler(QString errMsg, qint64 errCode)
{
    QMessageBox::critical(this, "Error " + QString::number(errCode),
                          errMsg);
}

void MainWindow::updateMotorsPower(quint8 motor1, quint8 motor2, quint8 motor3, quint8 motor4)
{
    motor1_power->setValue( motor1 );
    motor2_power->setValue( motor2 );
    motor3_power->setValue( motor3 );
    motor4_power->setValue( motor4 );
}
