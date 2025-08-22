#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSocketNotifier>
#include <QCoreApplication>
#include <QIcon>
#include "src/cpp/mqtt.h"




int main(int argc, char *argv[])
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;

    app.setWindowIcon(QIcon(":/Resources/zrrj8-cx3nc-001.ico"));
    qmlRegisterType<MQTT_WorkClass>("MyMqtt", 1, 0, "MQTT_WorkClass");

    QCoreApplication::setOrganizationName("MyCompany");
    QCoreApplication::setOrganizationDomain("mycompany.com");
    QCoreApplication::setApplicationName("MyApp");


    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
