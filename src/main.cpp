#include <QtQuick>

#include <sailfishapp.h>

#include "dicegeometry.h"
#include "dicetray.h"
#include "selftest.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName(QStringLiteral("org.hensip"));
    QCoreApplication::setApplicationName(QStringLiteral("harbour-saildice"));

    bool qmlCheck = false;
    bool autoRoll = false;
    bool showConfig = false;
    QString screenshotPath;
    for (int i = 1; i < argc; ++i) {
        const QString argument = QString::fromLocal8Bit(argv[i]);
        if (argument == QStringLiteral("--selftest")) {
            QCoreApplication app(argc, argv);
            return runSelfTest();
        } else if (argument == QStringLiteral("--qmlcheck")) {
            qmlCheck = true;
        } else if (argument == QStringLiteral("--roll")) {
            autoRoll = true;
        } else if (argument == QStringLiteral("--config")) {
            showConfig = true;
        } else if (argument.startsWith(QStringLiteral("--screenshot="))) {
            screenshotPath = argument.mid(QStringLiteral("--screenshot=").size());
        }
    }

    if (!screenshotPath.isEmpty() && qEnvironmentVariableIsEmpty("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("offscreen"));

    DiceGeometry::prebuild();

    qmlRegisterType<DiceTray>("harbour.saildice", 1, 0, "DiceTray");

    if (qmlCheck) {
        QCoreApplication app(argc, argv);
        QQmlEngine engine;
        const QString source = QCoreApplication::applicationDirPath() + QStringLiteral("/qml/harbour-saildice.qml");
        QQmlComponent component(&engine, QUrl::fromLocalFile(source));
        if (component.isError()) {
            for (const QQmlError &error : component.errors())
                QTextStream(stderr) << error.toString() << "\n";
            return 1;
        }
        QTextStream(stdout) << "QML loaded successfully\n";
        return 0;
    }

    QScopedPointer<QGuiApplication> app(SailfishApp::application(argc, argv));
    QScopedPointer<QQuickView> view(SailfishApp::createView());
    view->setResizeMode(QQuickView::SizeRootObjectToView);
    const QUrl source = SailfishApp::pathTo(QStringLiteral("qml/harbour-saildice.qml"));
    view->setSource(source);
    if (view->status() == QQuickView::Error) {
        for (const QQmlError &error : view->errors())
            QTextStream(stderr) << error.toString() << "\n";
        return 1;
    }

    if (!screenshotPath.isEmpty()) {
        view->resize(540, 960);
        view->rootObject()->setProperty("visible", true);
        view->setVisible(true);
        view->showFullScreen();
        view->requestActivate();
        if (autoRoll || showConfig) {
            QTimer::singleShot(600, [&]() {
                if (showConfig) {
                    QObject *page = view->rootObject()->findChild<QObject *>(QStringLiteral("mainPage"));
                    if (page)
                        QMetaObject::invokeMethod(page, "openConfig");
                }
                if (autoRoll)
                    QMetaObject::invokeMethod(view->rootObject(), "rollFromCover");
            });
        }
        QTimer::singleShot(autoRoll ? 4500 : 2500, [&]() {
            view->rootObject()->setProperty("visible", true);
            view->setVisible(true);
            view->requestActivate();
            QTimer::singleShot(250, [&]() {
                const QImage image = view->grabWindow();
                const bool saved = !image.isNull() && image.save(screenshotPath);
                QTextStream(stdout) << "Source: " << source.toString()
                                    << ", root: " << (view->rootObject() != nullptr)
                                    << ", visible: " << view->isVisible()
                                    << ", visibility: " << view->visibility()
                                    << ", rootVisible: " << view->rootObject()->property("visible").toBool()
                                    << ", state: " << QGuiApplication::applicationState()
                                    << ", view: " << view->width() << "x" << view->height()
                                    << ", image: " << image.width() << "x" << image.height() << "\n";
                QTextStream(stdout) << (saved ? "Screenshot saved: " : "Screenshot failed: ")
                                    << screenshotPath << "\n";
                app->exit(saved ? 0 : 1);
            });
        });
        return app->exec();
    }

    view->show();
    return app->exec();
}
