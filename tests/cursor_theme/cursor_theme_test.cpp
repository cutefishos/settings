#include "cursor/cursorthememodel.h"
#include <QDBusConnection>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QPixmap>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QTest>

#include <preferences.h>

class AppearanceService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.cutefish.Services.Appearance")
public slots:

    void setCursorTheme(const QString &theme)
    {
        QSettings settings(Preferences::file("services"), QSettings::IniFormat);
        settings.beginGroup("Appearance");
        settings.setValue("CursorTheme", theme);
        settings.sync();
        emit cursorThemeChanged();
    }

    void setCursorSize(int size)
    {
        QSettings settings(Preferences::file("services"), QSettings::IniFormat);
        settings.beginGroup("Appearance");
        settings.setValue("CursorSize", size);
        settings.sync();
        emit cursorSizeChanged();
    }

signals:
    void cursorThemeChanged();
    void cursorSizeChanged();
};

class CursorThemeTest : public QObject
{
    Q_OBJECT
private slots:

    void previewsAndServiceUpdates()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        qputenv("XCURSOR_PATH", directory.path().toUtf8());
        qputenv("XDG_CONFIG_HOME", directory.path().toUtf8());
        QVERIFY(QDir(directory.path()).mkpath("TestTheme/cursors"));
        QFile index(directory.filePath("TestTheme/index.theme"));
        QVERIFY(index.open(QIODevice::WriteOnly));
        // Icon-theme visibility must not hide bundled cursor themes such as Adwaita.
        index.write("[Icon Theme]\nName=Test Cursor\nHidden=true\n");
        index.close();
        QFile cursor(directory.filePath("TestTheme/cursors/left_ptr"));
        QVERIFY(cursor.open(QIODevice::WriteOnly));
        QDataStream out(&cursor);
        out.setByteOrder(QDataStream::LittleEndian);
        out << quint32(0x72756358) << quint32(16) << quint32(1) << quint32(1);
        out << quint32(0xfffd0002) << quint32(36) << quint32(28);
        out << quint32(36) << quint32(0xfffd0002) << quint32(36) << quint32(1);
        out << quint32(1) << quint32(1) << quint32(0) << quint32(0) << quint32(10)
            << quint32(0xff123456);
        cursor.close();

        AppearanceService service;
        auto bus = QDBusConnection::sessionBus();
        QVERIFY(bus.registerObject("/com/cutefish/Services/Appearance", &service,
                                   QDBusConnection::ExportAllSlots
                                           | QDBusConnection::ExportAllSignals));
        QVERIFY(bus.registerService("com.cutefish.Services"));
        CursorThemeModel first;
        CursorThemeModel second;
        const int row = first.themeIndex("TestTheme");
        QVERIFY(row >= 0);
        const auto preview =
                first.data(first.index(row), CursorThemeModel::ImageRole).value<QPixmap>();
        QVERIFY(!preview.isNull());
        QCOMPARE(preview.toImage().pixel(0, 0), QRgb(0xff123456));
        QSignalSpy changed(&second, &CursorThemeModel::currentThemeChanged);
        first.setCurrentTheme("TestTheme");
        QTRY_COMPARE(second.currentTheme(), QStringLiteral("TestTheme"));
        QVERIFY(!changed.isEmpty());
        first.setCursorSize(48);
        QTRY_COMPARE(second.cursorSize(), 48);
        first.setCurrentTheme("../missing");
        QCOMPARE(first.currentTheme(), QStringLiteral("TestTheme"));
        bus.unregisterService("com.cutefish.Services");
        bus.unregisterObject("/com/cutefish/Services/Appearance");
    }
};

int main(int argc, char **argv)
{
    QGuiApplication application(argc, argv);
    CursorThemeTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "cursor_theme_test.moc"
