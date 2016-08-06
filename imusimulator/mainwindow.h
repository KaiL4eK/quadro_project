#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "seriallinker.h"
#include "datastreamer.h"
#include "imudata.h"

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    DataStreamer *dataStream;

    bool connectionState = false;
    const qint32 anglesMultiplyer = 1000;
    IMUData data;

    int createStreamerThread();

signals:
    void stopDataStream();

public slots:
    void onConnectionBtnClick();
    void errorHandler( QString, qint64 );
    void updateIMURollData( double );
    void updateIMUPitchData( double );
    void changeConnectionState( bool state );

};

#endif // MAINWINDOW_H
