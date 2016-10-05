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

public slots:
    void createNewCurve();

private:
    uint16_t                    nCurves;

    QVector<QVector<double>>    *data_vect;
    QVector<QVector<double>>    *time_vect;
    QVector<QwtPlotCurve *>     *curves_vect;
};

class QwtPlotManager : public QObject
{
    Q_OBJECT

public:
    QwtPlotManager();
    ~QwtPlotManager();

    void addPlotWidget(uint16_t plotId, QString name, QString xAxisName = "Time", QString yAxisName = "Value");

    QWidget *getWidget();

    int testDataShow();
    int addPoint( QPointF *newPoint );
    int renderPlot();

    void refreshPlotView();

public slots:
    void redrawPlots();
    void redrawPlotIndex(uint16_t plotId);
    void addNewCurve(uint16_t plotId);

private:
    quint32                                     m_idCounter;
    quint16                                     m_widgetStringCounter;
    QVBoxLayout                                 *m_layout;
    QWidget                                     *m_widget;

    QMap<uint16_t, QwtStandartPlotWidget *>     ap_plots;

//    QTableView *pointsTable = NULL;
//    QStandardItemModel *tableModel = new QStandardItemModel( 0, 2 );
};

#endif // QWT_PLOTTER_H
