#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/** System header **/

#include <QMessageBox>

/** User headers **/
#include <controldata.h>
#include <devicebtselect.h>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void sendDataBtnClicked( bool clicked );

    void connectBtnPressed(bool clicked);
private:
    QWidget             *mainWgt;
    QVBoxLayout         *mainLayout;

    QGridLayout         *inputLayout;
    QWidget             *inputWgt;

    ControlData         *m_cData;

    QGridLayout         *controlLayout;
    QWidget             *controlWgt;
    QPushButton         *deviceConnectBtn;
    DeviceBTSelect      *deviceSelect;

    ControlDataView     *ratesRollPitch;
    ControlDataView     *ratesYaw;

    QPushButton         *send_data_btn;

    bool chooseConnectionDevice();
};

#endif // MAINWINDOW_H
