#ifndef PLOTMODEL_H
#define PLOTMODEL_H

#include <QObject>
#include <QAbstractSeries>
#include <QXYSeries>
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>
#include <QTimer>

QT_CHARTS_USE_NAMESPACE

class PlotModel : public QObject
{
    Q_OBJECT
public:
    explicit PlotModel(QObject *parent = nullptr);

    void resetDataCache();
    void newDataReceived(quint64 idx, QPointF &data);
    void setDataPeriod(quint32 dataPeriod);
    void setRenderPointCount(quint32 pointCount);

signals:

public slots:
    void updatePlotData(QAbstractSeries *series);


private:

    QVector<QPointF>    m_data;

    quint64             m_rangeMax;
    quint64             m_rangeMin;

    quint64             m_plotNumPoints;
    quint64             m_maxRangeX;
    quint32             m_stepRangeX_ms;
    qint32              m_rangeIdx;

    bool                m_isAxisUpdateRequired;
};

#endif // PLOTMODEL_H
