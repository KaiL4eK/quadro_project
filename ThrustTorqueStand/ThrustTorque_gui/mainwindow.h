#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QProgressBar>
#include <QLineEdit>
#include <QDebug>

#include "qwtPlotManager.h"

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
    QwtPlotManager plotMgr;

    /** UI references **/
    QPushButton *connectionBtn,
                *motorControlBtn;

    QLineEdit   *serialNameFld,
                *motorSpeedFld;

    QWidget     *controlWidget;

    bool connectionBtnState = false;

    void createNewLink();
    void setConnectedUIState();
    void setDisconnectedUIState();

signals:
    void stopDataLink();
    void callCalibration();
    void sendDataProcessingSignal(bool);
    void sendStartStopMotorSignal(bool, int);

public slots:
    void onHideBtnClicked(bool state);
    void onAboutBtnClicked();
    void onMotorStartBtnClick(bool state);
    void onSaveFileBtnClicked();
    void motorStartStopReady(bool completed);
    void onConnectionBtnClick(bool state);
    void changeConnectionState(bool state);
    void errorHandler(QString errMsg, qint64 errCode);
};

#endif // MAINWINDOW_H
