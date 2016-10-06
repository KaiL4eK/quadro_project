#ifndef QWT_PLOTTER_H
#define QWT_PLOTTER_H

#include <QString>
#include <QTableView>
#include <QStandardItemModel>
#include <QThread>
#include <QObject>
#include <QVBoxLayout>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>

#include <qwt_legend.h>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include <qwt_plot_magnifier.h>

#include <qwt_plot_panner.h>

#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

class QwtStandartPlotWidget : public QwtPlot
{
    Q_OBJECT

public:
    QwtStandartPlotWidget(QString name, QString xAxisName, QString yAxisName);
    ~QwtStandartPlotWidget();

    void redraw();
    void setDataSource(QVector<QVector<double>> *data_vect, QVector<QVector<double>> *time_vect);
    void createNewCurve();
    void clearPlotData();

private:
    quint16                    nCurves;

    QVector<QVector<double>>    *data_vect;
    QVector<QVector<double>>    *time_vect;
    QVector<QwtPlotCurve *>     curves_vect;
};

class QwtPlotManager : public QObject
{
    Q_OBJECT

public:
    QwtPlotManager();
    ~QwtPlotManager();

    void addPlotWidget(quint16 plotId, QString name, QString xAxisName = "Time", QString yAxisName = "Value");

    QWidget *getWidget();

    void clearPlots();

public slots:
    void redrawPlots();
    void redrawPlotIndex(quint16 plotId);
    void addNewCurve(quint16 plotId);
    void setDataSource(quint16 plotId, QVector<QVector<double>> *data_vect, QVector<QVector<double>> *time_vect);

private:
    QVBoxLayout                                 *m_layout;
    QWidget                                     *m_widget;

    QMap<quint16, QwtStandartPlotWidget *>      mpa_plot_widgets;

//    QTableView *pointsTable = NULL;
//    QStandardItemModel *tableModel = new QStandardItemModel( 0, 2 );
};

#endif // QWT_PLOTTER_H
