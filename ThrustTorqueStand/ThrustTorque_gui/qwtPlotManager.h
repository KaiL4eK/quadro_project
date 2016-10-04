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

class QwtStandartPlot : public QwtPlot
{

public:
    QwtStandartPlot(QString name, QString xAxisName, QString yAxisName);
    ~QwtStandartPlot();

private:
    QVector<QVector<double>> *data_vect;
    QVector<QVector<double>> *time_vect;
};

class QwtPlotManager : public QObject
{
    Q_OBJECT

public:
    QwtPlotManager();
    ~QwtPlotManager();

    void addPlotWidget(quint32 plotId, QString name, QString xAxisName = "Time", QString yAxisName = "Value");

    QWidget *getWidget();

    int testDataShow();
    int addPoint( QPointF *newPoint );
    int renderPlot();

    void refreshPlotView();

public slots:
    void dataProcess();

private:
    quint32                                     m_idCounter;
    quint16                                     m_widgetStringCounter;
    QVBoxLayout                                 *m_layout;
    QWidget                                     *m_widget;

    QMap<quint32, QwtStandartPlot *>            ap_plots;

//    QTableView *pointsTable = NULL;
//    QStandardItemModel *tableModel = new QStandardItemModel( 0, 2 );

//    QwtPlotCurve *mainCurve;
};

#endif // QWT_PLOTTER_H
