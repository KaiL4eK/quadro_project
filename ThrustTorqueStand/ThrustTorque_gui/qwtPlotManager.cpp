#include "qwtPlotManager.h"
#include <QApplication>

QwtPlotManager::QwtPlotManager()
    : m_layout( new QVBoxLayout() ), m_widget( new QWidget() )
{
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
}

QwtPlotManager::~QwtPlotManager()
{
}

void QwtPlotManager::addPlotWidget(quint16 plotId, QString name, QString xAxisName, QString yAxisName)
{
    if ( mpa_plot_widgets.contains( plotId ) )
        return;

    QwtStandartPlotWidget *newPlot = new QwtStandartPlotWidget(name, xAxisName, yAxisName);

    m_layout->addWidget( newPlot );

    mpa_plot_widgets.insert( plotId, newPlot );
}

QWidget *QwtPlotManager::getWidget()
{
    return( m_widget );
}

void QwtPlotManager::redrawPlotIndex(quint16 plotId)
{
    if ( !mpa_plot_widgets.contains( plotId ) )
        return;

    mpa_plot_widgets[plotId]->redraw();
}

void QwtPlotManager::redrawPlots()
{
    for( auto key: mpa_plot_widgets.keys() )
        mpa_plot_widgets.value( key )->redraw();
}

void QwtPlotManager::addNewCurve(quint16 plotId)
{
    if ( !mpa_plot_widgets.contains( plotId ) )
        return;

    mpa_plot_widgets[plotId]->createNewCurve();
}

void QwtPlotManager::setDataSource(quint16 plotId, QVector<QVector<double> > *data_vect, QVector<QVector<double> > *time_vect)
{
    if ( !mpa_plot_widgets.contains( plotId ) )
        return;

    mpa_plot_widgets[plotId]->setDataSource(data_vect, time_vect);
}

void QwtPlotManager::clearPlots()
{
    for( auto key: mpa_plot_widgets.keys() )
        mpa_plot_widgets.value( key )->clearPlotData();
}

/*
 *  QwtStandartPlot Methods
 */

QwtStandartPlotWidget::QwtStandartPlotWidget(QString name, QString xAxisName, QString yAxisName)
    : QwtPlot(), nCurves( 0 )
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
        qDebug() << "S-t bad in redraw function";

        QCoreApplication::processEvents();
        return;
    }

    if ( !data_vect || !time_vect ) {
        qDebug() << "Null pointers in redraw function";

        QCoreApplication::processEvents();
        return;
    }

    for ( quint16 i = 0; i < nCurves; i++ ) {
        curves_vect[i]->setRawSamples( time_vect->at(i).data(), data_vect->at(i).data(), data_vect->at(i).size() );
    }

    QCoreApplication::processEvents();
}

void QwtStandartPlotWidget::setDataSource(QVector<QVector<double> > *data_vect, QVector<QVector<double> > *time_vect)
{
    this->data_vect = data_vect;
    this->time_vect = time_vect;
}

void QwtStandartPlotWidget::createNewCurve()
{
    QwtPlotCurve *newCurve = new QwtPlotCurve( title().text() + " " + QString::number(curves_vect.size()) );

    newCurve->setPen( Qt::blue + curves_vect.size(), 1 );
    newCurve->setRenderHint( QwtPlotItem::RenderAntialiased, true );

    QwtSymbol *symbol = new QwtSymbol( QwtSymbol::Ellipse,
        QBrush( Qt::yellow ), QPen( Qt::blue, 1 ), QSize( 2, 2 ) );
    newCurve->setSymbol( symbol );
    newCurve->attach( this );

    curves_vect.append( newCurve );

    ++nCurves;
}

void QwtStandartPlotWidget::clearPlotData()
{
    for( auto it = curves_vect.begin(); it != curves_vect.end(); it++) {
        (*it)->detach();
        delete *it;
    }

    curves_vect.clear();
    nCurves = 0;
}
