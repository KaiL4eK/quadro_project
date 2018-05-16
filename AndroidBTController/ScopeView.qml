import QtQuick 2.9
import QtCharts 2.2


ChartView {

    id: chartView
    animationOptions: ChartView.NoAnimation
    theme: ChartView.ChartThemeDark
    property bool openGL: true
    property bool openGLSupported: true

    property var    plotModel
    property string chartName

    function updateData() {
        plotModel.updatePlotData(series(0));
    }

    antialiasing: true

//    onOpenGLChanged: {
//        if (openGLSupported) {
//            series(chartName).useOpenGL = openGL;
//            series("Reference").useOpenGL = openGL;
//        }
//    }
//    Component.onCompleted: {
//        if (!series("Signal").useOpenGL) {
//            openGLSupported = false
//            openGL = false
//        }
//    }

    ValueAxis {
        id: axisY
        min: -10
        max: 10
    }

    ValueAxis {
        id: axisX
        min: 0
        max: 1024
    }

    LineSeries {
        id: lineSeries1
        name: chartView.chartName
        axisX: axisX
        axisY: axisY
        useOpenGL: chartView.openGL
    }

    LineSeries {
        id: lineSeries2
        name: "Reference"
        axisX: axisX
        axisY: axisY
        useOpenGL: chartView.openGL
    }

//    transform: Scale {
//        id: scaler
//        origin.x: pinchArea.m_x2
//        origin.y: pinchArea.m_y2
//        xScale: pinchArea.m_zoom2
//        yScale: pinchArea.m_zoom2
//    }

    PinchArea {
        id: pinchArea
        anchors.fill: parent
//        property real m_x1: 0
//        property real m_y1: 0
//        property real m_y2: 0
//        property real m_x2: 0
//        property real m_zoom1: 0.5
//        property real m_zoom2: 0.5
//        property real m_max: 2
//        property real m_min: 0.5

//        onPinchStarted: {
//            console.log("Pinch Started")
//            m_x1 = scaler.origin.x
//            m_y1 = scaler.origin.y
//            m_x2 = pinch.startCenter.x
//            m_y2 = pinch.startCenter.y
//            rect.x = rect.x + (pinchArea.m_x1-pinchArea.m_x2)*(1-pinchArea.m_zoom1)
//            rect.y = rect.y + (pinchArea.m_y1-pinchArea.m_y2)*(1-pinchArea.m_zoom1)
//        }

//        onPinchUpdated: {
//            console.log("Pinch Updated")
//            m_zoom1 = scaler.xScale
//            var dz = pinch.scale-pinch.previousScale
//            var newZoom = m_zoom1+dz
//            if (newZoom <= m_max && newZoom >= m_min) {
//                m_zoom2 = newZoom
//            }
//        }

//        MouseArea {
//            id: dragArea
//            hoverEnabled: true
//            anchors.fill: parent
//            drag.target: chartView
//            drag.filterChildren: true
//        }

        //----------- old logic -----------
        onPinchUpdated: {
            chartView.zoomReset();

            var width_zoom = height/pinch.scale;
            var height_zoom = width/pinch.scale;

            var r = Qt.rect((2 * pinch.startCenter.x - pinch.center.x) - width_zoom/2, (2 * pinch.startCenter.y - pinch.center.y) - height_zoom/2, width_zoom, height_zoom)

            chartView.zoomIn(r)
        }
    }




}
