#include "qwtPlotManager.h"
#include <QApplication>

QwtPlotManager::QwtPlotManager()
    : m_idCounter( 0 ), m_widgetStringCounter( 0 )
{
    m_widget = new QWidget();
    m_layout = new QVBoxLayout();
    m_widget->setLayout( m_layout );

    // Initialize table
//    tableModel->setHorizontalHeaderItem( 0, new QStandardItem( QString("Time") ) );
//    tableModel->setHorizontalHeaderItem( 1, new QStandardItem( QString("Angle") ) );

//    pointsTable->setModel( tableModel );
//    pointsTable->setColumnWidth( 0, 90 );
//    pointsTable->setColumnWidth( 1, 90 );
//    QFont *tableFont = new QFont( "Arial" );
//    tableFont->setPixelSize( 15 );
//    pointsTable->setFont( *tableFont );

    // Curve

}

QwtPlotManager::~QwtPlotManager()
{
}

void QwtPlotManager::addPlotWidget(uint16_t plotId, QString name, QString xAxisName, QString yAxisName)
{
    if ( ap_plots.contains( plotId ) )
        return;

    QwtStandartPlotWidget *newPlot = new QwtStandartPlotWidget(name, xAxisName, yAxisName);

    m_layout->addWidget( newPlot );

    ap_plots.insert( plotId, newPlot );
}

QWidget *QwtPlotManager::getWidget()
{
    return( m_widget );
}

//int qwtPlotter::testDataShow( void )
//{
//    addPoint( new QPointF( 1.0, 1.0 ) );
//    addPoint( new QPointF( 1.5, 2.0 ) );
//    addPoint( new QPointF( 3.0, 2.0 ) );
//    addPoint( new QPointF( 3.5, 3.0 ) );
//    addPoint( new QPointF( 5.0, 3.0 ) );

//    return( 0 );
//}

//int qwtPlotter::addPoint( QPointF *newPoint )
//{
//    dataList.push_back( *newPoint );
//    int rowAddIndex = tableModel->rowCount();
//    tableModel->setItem( rowAddIndex, 0, new QStandardItem( QString::number( newPoint->x() ) ) );
//    tableModel->setItem( rowAddIndex, 1, new QStandardItem( QString::number( newPoint->y() ) ) );
//    pointsTable->setRowHeight( rowAddIndex, 20 );

//    return( 0 );
//}

void QwtPlotManager::redrawPlotIndex(uint16_t plotId)
{
    if ( !ap_plots.contains( plotId ) )
        return;

    ap_plots[plotId]->redraw();
}

void QwtPlotManager::redrawPlots()
{
    for ( const auto &kv : ap_plots ) {
        kv->redraw();
    }
}

void QwtPlotManager::addNewCurve(uint16_t plotId)
{
    if ( !ap_plots.contains( plotId ) )
        return;

    ap_plots[plotId]->createNewCurve();
}

void QwtPlotManager::setDataSource(uint16_t plotId, QVector<QVector<double> > *data_vect, QVector<QVector<double> > *time_vect)
{
    if ( !ap_plots.contains( plotId ) )
        return;

    ap_plots[plotId]->setDataSource(data_vect, time_vect);
}

/*
 *  QwtStandartPlot Methods
 */

QwtStandartPlotWidget::QwtStandartPlotWidget(QString name, QString xAxisName, QString yAxisName)
    : QwtPlot()
{
    QwtPlotGrid *grid       = new QwtPlotGrid();

    // Initialize plot
    setTitle( name );
    setAxisTitle( QwtPlot::yLeft, yAxisName );
    setAxisTitle( QwtPlot::xBottom, xAxisName );
    setAutoReplot();
    insertLegend( new QwtLegend, QwtPlot::BottomLegend );

    grid->setMajorPen( QPen( Qt::gray, 1 ) );
    grid->attach( this );

    // Mouse handlers
    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier( canvas() );
    magnifier->setMouseButton( Qt::MidButton );

    QwtPlotPanner *d_panner = new QwtPlotPanner( canvas() );
    d_panner->setMouseButton( Qt::RightButton );

    QwtPlotPicker *d_picker = new QwtPlotPicker (QwtPlot::xBottom, QwtPlot::yLeft, // ассоциация с осями
                                                 QwtPlotPicker::CrossRubberBand, // стиль перпендикулярных линий
                                                 QwtPicker::ActiveOnly, // включение/выключение
                                                 canvas());

    d_picker->setRubberBandPen( QColor( Qt::red ) );
    d_picker->setTrackerPen( QColor( Qt::black ) );
    d_picker->setStateMachine( new QwtPickerDragPointMachine() );

}

QwtStandartPlotWidget::~QwtStandartPlotWidget()
{

}

void QwtStandartPlotWidget::redraw()
{
    setAxisAutoScale(QwtPlot::yLeft);
    setAxisAutoScale(QwtPlot::xBottom);

    if ( nCurves > data_vect->size() ||
         nCurves > time_vect->size() ||
         nCurves != curves_vect.size() ) {
        // Error processing
        return;
    }

    for ( uint16_t i = 0; i < nCurves; i++ ) {
        curves_vect[i]->setRawSamples( time_vect->at(i).data(), data_vect->at(i).data(), data_vect->at(i).size() );
    }
}

void QwtStandartPlotWidget::setDataSource(QVector<QVector<double> > *data_vect, QVector<QVector<double> > *time_vect)
{
    this->data_vect = data_vect;
    this->time_vect = time_vect;
}

void QwtStandartPlotWidget::createNewCurve()
{
    QwtPlotCurve *newCurve = new QwtPlotCurve( title().text() + " " + curves_vect.size() );

    newCurve->setPen( Qt::blue + curves_vect.size(), 1 );
    newCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
        QBrush( Qt::yellow ), QPen( Qt::blue, 1 ), QSize( 2, 2 ) );
    newCurve->setSymbol( symbol );
    newCurve->attach( this );

    curves_vect.append( newCurve );

    ++nCurves;
}
