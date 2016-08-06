#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QThread>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect( ui->connectionBtn, SIGNAL(clicked(bool)), this, SLOT(onConnectionBtnClick()) );
    connect( ui->pitchSetAngleDgtl, SIGNAL(valueChanged(double)), this, SLOT(updateIMUPitchData(double)) );
    connect( ui->pitchSetAngleKnb, SIGNAL(valueChanged(double)), this, SLOT(updateIMUPitchData(double)) );
    connect( ui->rollSetAngleDgtl, SIGNAL(valueChanged(double)), this, SLOT(updateIMURollData(double)) );
    connect( ui->rollSetAngleKnb, SIGNAL(valueChanged(double)), this, SLOT(updateIMURollData(double)) );
}

MainWindow::~MainWindow()
{
    delete ui;
}

int MainWindow::createStreamerThread()
{
    qDebug() << "Registering new stream from " << QThread::currentThreadId();
    QThread *streamThread = new QThread;
    SerialLinker *link = new SerialLinker( ui->serialNameFld->text(),
                                           ui->serialSpeedBox->currentText().toInt()
                                           );
    dataStream = new DataStreamer( link, &data );
    dataStream->moveToThread( streamThread );

    // Error handling subscribe
    connect( link, SIGNAL(error(QString, qint64)), this, SLOT(errorHandler(QString, qint64)) );
    connect( dataStream, SIGNAL(error(QString, qint64)), this, SLOT(errorHandler(QString, qint64)) );

    connect( streamThread, SIGNAL(started()), dataStream, SLOT(process()) );

    connect( this, SIGNAL(stopDataStream()), dataStream, SLOT(stopStream()), Qt::DirectConnection );
    connect( dataStream, SIGNAL(finished()), streamThread, SLOT(quit()) );

    // Self delete
    connect( dataStream, &DataStreamer::finished, dataStream, &DataStreamer::deleteLater );
    connect( streamThread, SIGNAL(finished()), streamThread, SLOT(deleteLater()) );

    connect( dataStream, &DataStreamer::sendConnectionState, this, &MainWindow::changeConnectionState );

    streamThread->start();

    return( 0 );
}

void MainWindow::onConnectionBtnClick()
{
    if ( !connectionState )
    {   // Connect to the port
        createStreamerThread();
    }
    else
    {   // Disconnect from the port
        qDebug() << "Try to stop thread from " << QThread::currentThreadId();
        emit stopDataStream();
    }
}

void MainWindow::errorHandler(QString errMsg, qint64 errCode)
{
    QMessageBox::critical(this, "Error " + QString::number(errCode), errMsg);
}

void MainWindow::updateIMURollData( double newRoll )
{
    data.rollAngle1000 = newRoll*anglesMultiplyer;
}

void MainWindow::updateIMUPitchData( double newPitch )
{
    data.pitchAngle1000 = newPitch*anglesMultiplyer;
}

void MainWindow::changeConnectionState( bool state )
{
    connectionState = state;
    if ( connectionState )
        ui->connectionBtn->setText( "Disconnect" );
    else
        ui->connectionBtn->setText( "Connect" );
}
