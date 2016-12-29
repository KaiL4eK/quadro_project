#include "qwtPlotter.h"
#include <QApplication>

QwtPlotter::QwtPlotter( QwtPlot *plot, QVector<double> *dbPtr, QVector<double> *dbEncPtr, QVector<double> *timeVectPtr, QString angleName )
    : qwtPlot( plot ), dataList( dbPtr ),
      encDataList( dbEncPtr ), timeList( timeVectPtr ),
      plotName( angleName )
{
    // Initialize plot
    qwtPlot->setTitle( plotName + " chart" );
    qwtPlot->setAxisTitle( QwtPlot::yLeft, "Angle" );
    qwtPlot->setAxisTitle( QwtPlot::xBottom, "Time, ms" );
    qwtPlot->setAutoReplot();
    qwtPlot->insertLegend( new QwtLegend, QwtPlot::BottomLegend );

    QwtPlotGrid *grid = new QwtPlotGrid();
    grid->setMajorPen( QPen( Qt::gray, 1 ) );
    grid->attach( qwtPlot );

    // Handlers
    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier( qwtPlot->canvas() );
    magnifier->setMouseButton( Qt::MidButton );

    QwtPlotPanner *d_panner = new QwtPlotPanner( qwtPlot->canvas() );
    d_panner->setMouseButton( Qt::RightButton );

    QwtPlotPicker *d_picker = new QwtPlotPicker (QwtPlot::xBottom, QwtPlot::yLeft, // ассоциация с осями
                                                 QwtPlotPicker::CrossRubberBand, // стиль перпендикулярных линий
                                                 QwtPicker::ActiveOnly, // включение/выключение
                                                 qwtPlot->canvas());

    d_picker->setRubberBandPen( QColor( Qt::red ) );
    d_picker->setTrackerPen( QColor( Qt::black ) );
    d_picker->setStateMachine( new QwtPickerDragPointMachine() );

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
    mainCurve = new QwtPlotCurve(plotName);
    mainCurve->setPen( Qt::blue, 1 );
    mainCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
        QBrush( Qt::yellow ), QPen( Qt::blue, 1 ), QSize( 2, 2 ) );
    mainCurve->setSymbol( symbol );
    mainCurve->attach( qwtPlot );

    encoderCurve = new QwtPlotCurve("Encoder " + plotName);
    encoderCurve->setPen( Qt::red, 1 );
    encoderCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    symbol = new QwtSymbol( QwtSymbol::Ellipse,
        QBrush( Qt::yellow ), QPen( Qt::red, 1 ), QSize( 2, 2 ) );
    encoderCurve->setSymbol( symbol );
    encoderCurve->attach( qwtPlot );
}

QwtPlotter::~QwtPlotter()
{

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

int QwtPlotter::renderPlot()
{
    mainCurve->setRawSamples( timeList->data(), dataList->data(), dataList->size() );
    encoderCurve->setRawSamples( timeList->data(), encDataList->data(), encDataList->size() );
    return( 0 );
}

void QwtPlotter::refreshPlotView()
{
    qwtPlot->setAxisAutoScale(QwtPlot::yLeft);
    qwtPlot->setAxisAutoScale(QwtPlot::xBottom);
}

void QwtPlotter::dataProcess()
{
    renderPlot();
    QCoreApplication::processEvents();
}
