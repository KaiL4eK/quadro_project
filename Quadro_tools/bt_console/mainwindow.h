#ifndef MAINWINDOW_H
#define MAINWINDOW_H

/** System header **/
#include <QMainWindow>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>

#include <QDoubleValidator>
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
    void p_pos_rate_btn_clicked( bool clicked );

    void rate_flds_edit_fin();

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

    QLineEdit   *p_pos_rate_fld;
    QLineEdit   *i_pos_rate_fld;
    QLineEdit   *p_spd_rate_fld;
    QLineEdit   *d_spd_rate_fld;

    QPushButton *p_pos_rate_set_btn;
    QPushButton *i_pos_rate_set_btn;
    QPushButton *p_spd_rate_set_btn;
    QPushButton *d_spd_rate_set_btn;


    bool chooseConnectionDevice();
};

#endif // MAINWINDOW_H
