#include "mainwindow.h"

#include <QObject>
#include <QPixmap>
#include <QLabel>
#include <QMessageBox>
#include <QLineEdit>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    connectionBtn = new QPushButton(tr("Connect"));
//    connectionBtn->setMinimumHeight(55);
    connectionBtn->setCheckable(true);
    connect( connectionBtn,    &QPushButton::clicked, this, &MainWindow::onConnectionBtnClick );

    startMeasureBtn = new QPushButton("Start measure");
    startMeasureBtn->setCheckable(true);
    connect( startMeasureBtn,  &QPushButton::clicked, this, &MainWindow::onMeasureStartBtnClick );

    serialNameFld = new QLineEdit();
    serialNameFld->setText(tr("/dev/ttyUSB0"));
    motorPowerStart = new QLineEdit();
//    motorPowerStart->setText("0");
    motorPowerStart->setPlaceholderText(QString("Set start power, %"));
    motorPowerEnd = new QLineEdit();
//    motorPowerEnd->setText("0");
    motorPowerEnd->setPlaceholderText(QString("Set end power, %"));
    timeMeasureDeltaMs = new QLineEdit();
//    timeMeasureDeltaMs->setText(tr("1000"));
    timeMeasureDeltaMs->setPlaceholderText(QString("Set full measure time, ms"));
    timeMeasureStartMs = new QLineEdit();
//    timeMeasureStartMs->setText(tr("1000"));
    timeMeasureStartMs->setPlaceholderText(QString("Set time offset, ms"));
    timeStepMs = new QLineEdit();
//    timeStepMs->setText("0");
    timeStepMs->setPlaceholderText(QString("Set time to change power, ms"));

    QPushButton *aboutBtn = new QPushButton(tr("About"));
    connect( aboutBtn, &QPushButton::clicked, this, &MainWindow::onAboutBtnClicked );

    saveFileBtn = new QPushButton(tr("Save plots"));
    connect( saveFileBtn, &QPushButton::clicked, this, &MainWindow::onSaveFileBtnClicked );

    setParamsBtn = new QPushButton(tr("Set params"));
    connect( setParamsBtn, &QPushButton::clicked, this, &MainWindow::onSetParamsBtnClicked );

    QGridLayout *controlLayout = new QGridLayout();
    controlWidget = new QWidget(this);
    controlWidget->setLayout(controlLayout);
    controlLayout->addWidget(connectionBtn, 0, 0);
    controlLayout->addWidget(serialNameFld, 0, 1, 1, 5);
    controlLayout->addWidget(aboutBtn, 0, 6, Qt::AlignBottom|Qt::AlignRight);
    controlLayout->addWidget(saveFileBtn, 0, 7, Qt::AlignBottom|Qt::AlignRight);

    controlLayout->addWidget(startMeasureBtn, 1, 0);
    controlLayout->addWidget(motorPowerStart, 1, 1);
    controlLayout->addWidget(motorPowerEnd, 1, 2);
    controlLayout->addWidget(timeMeasureStartMs, 1, 3);
    controlLayout->addWidget(timeStepMs, 1, 4);
    controlLayout->addWidget(timeMeasureDeltaMs, 1, 5);
    controlLayout->addWidget(setParamsBtn, 1, 6);

    QPushButton *hideBtn = new QPushButton();
    hideBtn->setCheckable(true);
    hideBtn->setMaximumHeight(20);

    QGridLayout *layout = new QGridLayout;

    plotMgr.addPlotWidget( ThrustDataIndex, "Thrust plot", "Time, ms", "Value, g" );
    plotMgr.addPlotWidget( TorqueDataIndex, "Torque plot", "Time, ms", "Value, Nm" );
    plotMgr.addPlotWidget( CurrentDataIndex, "Current plot", "Time, ms", "Value, mA" );
    plotMgr.addPlotWidget( SpeedDataIndex, "Speed plot", "Time, ms", "Value, rpm" );
    layout->addWidget( plotMgr.getWidget(), 0, 0 );

    layout->addWidget(hideBtn, 1, 0);
    layout->addWidget(controlWidget, 2, 0);


    QWidget *window = new QWidget();
    window->setLayout(layout);
    window->setMinimumSize( QSize(600, 800) );
    setCentralWidget(window);
    setWindowTitle(tr("Thrust Stand"));

    setDisconnectedUIState();

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
    SerialLink *serial = new SerialLink( serialNameFld->text() );
    serial->moveToThread( linkThread );

    connect( serial, &SerialLink::error, this, &MainWindow::errorHandler );
    connect( serial, &SerialLink::dataReceived, &plotMgr, &QwtPlotManager::redrawPlots );
    connect( serial, &SerialLink::setDataSource, &plotMgr, &QwtPlotManager::setDataSource );
    connect( serial, &SerialLink::addNewCurve, &plotMgr, &QwtPlotManager::addNewCurve );

    connect( this, &MainWindow::stopDataLink, serial, &SerialLink::stopLink );

    connect( linkThread, SIGNAL(started()), serial, SLOT(process()) );
    connect( serial, SIGNAL(finished()), linkThread, SLOT(quit()) );
    connect( serial, &SerialLink::finished, serial, &SerialLink::deleteLater );
    connect( linkThread, SIGNAL(finished()), linkThread, SLOT(deleteLater()) );

    connect( serial, &SerialLink::sendConnectionState, this, &MainWindow::changeConnectionState );

    connect( this, &MainWindow::sendMeasureStartSignal, serial, &SerialLink::processStartStopMotorCommand );
    connect( this, &MainWindow::sendSetParamsSignal, serial, &SerialLink::processSetParametersCommand );
    connect( serial, &SerialLink::sendMotorStartStopFinished, this, &MainWindow::motorStartStopReady );

    linkThread->start();
}

void MainWindow::setConnectedUIState()
{
    connectionBtn->setChecked(true);
    serialNameFld->setEnabled(false);
    startMeasureBtn->setEnabled(true);
    setParamsBtn->setEnabled(true);
    saveFileBtn->setEnabled(true);

    motorPowerStart->setEnabled(true);
    motorPowerEnd->setEnabled(true);
    timeMeasureDeltaMs->setEnabled(true);
    timeStepMs->setEnabled(true);
    timeMeasureStartMs->setEnabled(true);
}

void MainWindow::setDisconnectedUIState()
{
    connectionBtn->setChecked(false);
    serialNameFld->setEnabled(true);
    startMeasureBtn->setEnabled(false);
    startMeasureBtn->setChecked(false);
    setParamsBtn->setEnabled(false);
    saveFileBtn->setEnabled(false);

    motorPowerStart->setEnabled(false);
    motorPowerEnd->setEnabled(false);
    timeMeasureDeltaMs->setEnabled(false);
    timeStepMs->setEnabled(false);
    timeMeasureStartMs->setEnabled(false);
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

void MainWindow::onMeasureStartBtnClick(bool state)
{
    emit sendMeasureStartSignal(state);
    if ( state )
        startMeasureBtn->setEnabled(false);
    else
        startMeasureBtn->setEnabled(true);
}

void MainWindow::onSaveFileBtnClicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home/alexey",
                                                    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    plotMgr.saveDataToDirectory( dir );
}

void MainWindow::onSetParamsBtnClicked()
{
    if ( motorPowerStart->text().isEmpty()      ||
         motorPowerEnd->text().isEmpty()        ||
         timeStepMs->text().isEmpty()           ||
         timeMeasureStartMs->text().isEmpty()   ||
         timeMeasureDeltaMs->text().isEmpty() ) {
        errorHandler( "Parameters are not set", -1 );
    }

    MeasureParams str;

    str.motor_power_start       = motorPowerStart->text().toShort();
    str.motor_power_stop        = motorPowerEnd->text().toShort();
    str.time_step_moment_ms     = timeStepMs->text().toInt();
    str.time_measure_start_ms   = timeMeasureStartMs->text().toInt();
    str.time_measure_ms         = timeMeasureDeltaMs->text().toInt();

    emit sendSetParamsSignal( str );
}

void MainWindow::motorStartStopReady()
{
    startMeasureBtn->setEnabled(false);
}

void MainWindow::changeConnectionState( bool state )
{
    if ( state ) { // Now connected
        connectionBtn->setText( "Disconnect" );
        setConnectedUIState();
    } else { // Now disconnected
        connectionBtn->setText( "Connect" );
        setDisconnectedUIState();
    }

    connectionBtn->setEnabled( true );
}

void MainWindow::onConnectionBtnClick(bool state)
{
    connectionBtn->setEnabled( false );
    if ( state ) { // Was Disconnected
        createNewLink();
    } else { // Was Connected
        emit stopDataLink();
        plotMgr.clearPlots();
    }
}

void MainWindow::errorHandler(QString errMsg, qint64 errCode)
{
    QMessageBox::critical(this, "Error " + QString::number(errCode), errMsg);
}
