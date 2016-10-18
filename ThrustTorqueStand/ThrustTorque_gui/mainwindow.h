#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QProgressBar>
#include <QLineEdit>
#include <QDebug>

#include "serialLink.h"
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
                *startMeasureBtn,
                *setParamsBtn,
                *saveFileBtn;

    QLineEdit   *serialNameFld,
                *motorPowerStart,
                *motorPowerEnd,
                *timeMeasureDeltaMs,
                *timeStepMs,
                *timeMeasureStartMs;

    QWidget     *controlWidget;

    bool connectionBtnState = false;

    void createNewLink();
    void setConnectedUIState();
    void setDisconnectedUIState();

signals:
    void stopDataLink();
    void callCalibration();
    void sendDataProcessingSignal(bool);
    void sendMeasureStartSignal(bool);
    void sendSetParamsSignal(MeasureParams);

public slots:
    void onHideBtnClicked(bool state);
    void onAboutBtnClicked();
    void onMeasureStartBtnClick(bool state);
    void onSaveFileBtnClicked();
    void onSetParamsBtnClicked();
    void motorStartStopReady();
    void onConnectionBtnClick(bool state);
    void changeConnectionState(bool state);
    void errorHandler(QString errMsg, qint64 errCode);
};

#endif // MAINWINDOW_H
