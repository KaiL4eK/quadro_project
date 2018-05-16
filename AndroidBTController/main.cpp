#include <QApplication>

#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QQmlContext>

#include <backend.h>
#include <pid_controller_model.h>

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    Backend             backend;

    QApplication app(argc, argv);

//    qmlRegisterType<Backend>("Backend", 1, 0, "Backend");

    QQmlApplicationEngine engine;

    QQmlContext *ctxt = engine.rootContext();
    ctxt->setContextProperty("mainRatesModel", &backend.m_ratesRollPitchModel);
    ctxt->setContextProperty("yawRatesModel", &backend.m_ratesYawModel);
    ctxt->setContextProperty("rollPlotModel", &backend.m_rollModel);
    ctxt->setContextProperty("pitchPlotModel", &backend.m_pitchModel);
    ctxt->setContextProperty("backend", &backend);

    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
