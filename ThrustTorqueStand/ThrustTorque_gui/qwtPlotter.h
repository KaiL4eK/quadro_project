#ifndef QWT_PLOTTER_H
#define QWT_PLOTTER_H

#include <QString>
#include <QTableView>
#include <QStandardItemModel>
#include <QThread>
#include <QObject>

#include <qwt_plot.h>
#include <qwt_plot_grid.h>

#include <qwt_legend.h>

#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

#include <qwt_plot_magnifier.h>

#include <qwt_plot_panner.h>

#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

class QwtPlotter : public QObject
{
    Q_OBJECT

public:
    QwtPlotter( QwtPlot *plot, QVector<double> *dbPtr, QVector<double> *timeVectPtr, QString plotName );
    ~QwtPlotter();
    int testDataShow();
    int addPoint( QPointF *newPoint );
    int renderPlot();

    void refreshPlotView();

public slots:
    void dataProcess();

private:
    QwtPlot *qwtPlot = NULL;
//    QTableView *pointsTable = NULL;
    QVector<double> *dataList = NULL,
                    *encDataList = NULL,
                    *timeList = NULL;
//    QStandardItemModel *tableModel = new QStandardItemModel( 0, 2 );

    QwtPlotCurve *mainCurve;
    QString plotName;



};

#endif // QWT_PLOTTER_H
