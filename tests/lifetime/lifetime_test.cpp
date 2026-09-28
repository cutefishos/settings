// Page backends are created when their page opens and destroyed when it closes.
// Built with AddressSanitizer, so memory one leaves behind fails the run at exit.

#include "datetime/timezonemap.h"
#include "docksettings.h"
#include "fontsmodel.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>

#include <algorithm>

#include <fontconfig/fontconfig.h>
#include <sanitizer/lsan_interface.h>

class LifetimeTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // fontconfig keeps its configuration for the whole process; loading it
        // untracked leaves only what a font scan leaves behind to count.
        __lsan::ScopedDisabler disabler;
        QVERIFY(FcInit());
    }

    void fontsModelHandsItsListsToItsOwnThread()
    {
        FontsModel model;
        QThread *notifier = nullptr;
        connect(&model, &FontsModel::loadFinished, this,
                [&notifier] { notifier = QThread::currentThread(); }, Qt::DirectConnection);
        QVERIFY(model.wait());

        // The scan is over, but the lists change only on the thread QML reads them on.
        QVERIFY(model.generalFonts().isEmpty());
        QVERIFY(model.fixedFonts().isEmpty());
        QTRY_COMPARE(notifier, QThread::currentThread());

        const QStringList general = model.generalFonts();
        const QStringList fixed = model.fixedFonts();
        QVERIFY(!general.isEmpty() || !fixed.isEmpty());
        QVERIFY(std::is_sorted(general.cbegin(), general.cend()));
        QVERIFY(std::is_sorted(fixed.cbegin(), fixed.cend()));
    }

    void fontsModelClosedBeforeItsListsArrive()
    {
        auto *model = new FontsModel;
        QVERIFY(model->wait());
        delete model;

        // The queued hand-off must go with the model.
        QCoreApplication::processEvents();
    }

    void dockSettingsWritesOutWhenDestroyed()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        QSettings::setPath(QSettings::NativeFormat, QSettings::UserScope, directory.path());
        QVERIFY(QDir(directory.path()).mkpath("cutefish"));
        QFile file(directory.filePath("cutefish/dock.conf"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("[General]\nStyle=1\n");
        file.close();

        // Its settings object goes with it and writes the Style migration out.
        auto *dock = new DockSettings;
        delete dock;

        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().contains("Style=0"));
    }

    void timeZoneMapFindsAZoneAtItsOwnSpot()
    {
        TimeZoneMap map;
        const QString zone = QStringLiteral("Asia/Singapore");
        const QPointF point = map.position(zone, 3600, 1800);
        QVERIFY(point.x() >= 0 && point.y() >= 0);
        QCOMPARE(map.nearest(qRound(point.x()), qRound(point.y()), 3600, 1800), zone);
    }
};

QTEST_GUILESS_MAIN(LifetimeTest)

#include "lifetime_test.moc"
