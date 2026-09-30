#include "cursor/mouse.h"
#include "input/mouse_settings.h"

#include <QCoreApplication>
#include <QProcess>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

class MouseIntegrationTest : public QObject
{
    Q_OBJECT

private slots:

    void startupNotificationsAndRestart()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        Mouse first;
        Mouse second;
        QVERIFY(!first.available());
        QSignalSpy handedness(&second, &Mouse::leftHandedChanged);
        QSignalSpy scrolling(&second, &Mouse::naturalScrollChanged);
        QSignalSpy acceleration(&second, &Mouse::accelerationChanged);
        QSignalSpy speed(&second, &Mouse::pointerAccelerationChanged);

        QProcess server;
        server.setProgram(QCoreApplication::applicationFilePath());
        server.setArguments(
                {QStringLiteral("--serve"), directory.filePath(QStringLiteral("mouse.conf"))});
        server.start();
        QVERIFY(server.waitForStarted());
        QTRY_VERIFY(first.available());
        QTRY_VERIFY(second.available());
        handedness.clear();
        scrolling.clear();
        acceleration.clear();
        speed.clear();

        first.setLeftHanded(true);
        first.setNaturalScroll(false);
        first.setAcceleration(false);
        first.setPointerAcceleration(0.6);
        QTRY_VERIFY(!handedness.isEmpty());
        QTRY_VERIFY(!scrolling.isEmpty());
        QTRY_VERIFY(!acceleration.isEmpty());
        QTRY_VERIFY(!speed.isEmpty());
        QVERIFY(second.leftHanded());
        QVERIFY(!second.naturalScroll());
        QVERIFY(!second.acceleration());
        QCOMPARE(second.pointerAcceleration(), 0.6);

        server.terminate();
        QVERIFY(server.waitForFinished());
        QTRY_VERIFY(!first.available());
        QTRY_VERIFY(!second.available());
        server.start();
        QVERIFY(server.waitForStarted());
        QTRY_VERIFY(first.available());
        QTRY_VERIFY(second.available());
        QVERIFY(first.leftHanded());
        QVERIFY(!first.naturalScroll());
        QVERIFY(!first.acceleration());
        QCOMPARE(first.pointerAcceleration(), 0.6);

        second.setAcceleration(true);
        QTRY_VERIFY(first.acceleration());
        server.terminate();
        QVERIFY(server.waitForFinished());
    }
};

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    if (application.arguments().value(1) == QStringLiteral("--serve")) {
        cutefish::input::MouseSettings settings(nullptr, application.arguments().value(2));
        settings.setAvailable(true);
        if (!settings.registerService())
            return 1;
        return application.exec();
    }
    MouseIntegrationTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "mouse_integration_test.moc"
