#include "plotmodel.h"

PlotModel::PlotModel(QObject *parent) : QObject(parent),
    m_plotNumPoints(1000), m_stepRangeX_ms(10), m_isAxisUpdateRequired(false)
{
    m_maxRangeX     = m_plotNumPoints * m_stepRangeX_ms;
    resetDataCache();
}

void PlotModel::resetDataCache()
{
    m_rangeIdx = -1;

    m_data.clear();
    m_data << QPointF(0, 0);
}

void PlotModel::newDataReceived(quint64 idx, QPointF &data)
{
    qint32 diff = idx - (m_data.length() - 1);

    if ( diff > 0 )
    {
//        qDebug() << "Difference in length() " << diff;

        while ( diff )
        {
            QPointF &last = m_data.last();
            last.rx() += m_stepRangeX_ms;

            m_data << last;
            diff--;
        }
    }

    m_data << data;
}

void PlotModel::setDataPeriod(quint32 dataPeriod)
{
    m_stepRangeX_ms = dataPeriod;

    m_maxRangeX     = m_plotNumPoints * m_stepRangeX_ms;
    m_rangeIdx      = (int)(m_data.last().x()) / m_maxRangeX;
    m_isAxisUpdateRequired = true;
}

void PlotModel::setRenderPointCount(quint32 pointCount)
{
    m_plotNumPoints = pointCount;

    m_maxRangeX     = m_plotNumPoints * m_stepRangeX_ms;
    m_rangeIdx      = (int)(m_data.last().x()) / m_maxRangeX;
    m_isAxisUpdateRequired = true;
}

void PlotModel::updatePlotData(QAbstractSeries *series)
{
    if (series)
    {
        QXYSeries *xySeries = static_cast<QXYSeries *>(series);

//        qDebug() << ">>> Update start! " + QString::number( m_rangeIdx );

        if ( m_data.last().x() > (m_rangeIdx + 1) * m_maxRangeX )
        {
            m_rangeIdx++;
            m_isAxisUpdateRequired = true;
        }

        if ( m_isAxisUpdateRequired )
        {
            m_rangeMin = m_maxRangeX * m_rangeIdx;
            m_rangeMax = m_maxRangeX * (m_rangeIdx + 1);

            xySeries->attachedAxes()[0]->setRange( QVariant( m_rangeMin ), QVariant( m_rangeMax ) );

            qDebug() << "Range " << m_rangeMin << " " << m_rangeMax;
            m_isAxisUpdateRequired = false;
        }

//        quint64 left_range = m_rangeIdx * m_plotNumPoints;
//        QVector<QPointF> points = m_data.mid( left_range, m_data.length() - left_range );
        // Use replace instead of clear + append, it's optimized for performance
        xySeries->replace( m_data );
    }
}

