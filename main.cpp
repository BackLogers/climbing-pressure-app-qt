#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "applogic.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    AppLogic appLogic;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("appLogic", &appLogic);
    const QUrl url(u"qrc:/ScaleQuick/main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
