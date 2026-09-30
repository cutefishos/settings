#include "mimeapps.h"
#include "defaultapplications.h"
#include "applicationregistry.h"
#include <QFile>
#include <QSettings>
#include <QTemporaryDir>
#include <QtTest>

class MimeAppsTest : public QObject
{
    Q_OBJECT
private slots:

    void settingsWritesStandardAssociations()
    {
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        qputenv("XDG_CONFIG_HOME", dir.path().toUtf8());
        qputenv("XDG_DATA_HOME", dir.path().toUtf8());
        qputenv("XDG_DATA_DIRS", dir.path().toUtf8());
        const QString path = dir.filePath("mimeapps.list");
        {
            QSettings legacy(path, QSettings::IniFormat);
            legacy.beginGroup("Default Applications");
            legacy.setValue("x-scheme-handler/http", "other.desktop;");
        }

        auto *registry = ApplicationRegistry::instance();
        QTRY_VERIFY(!registry->loading());
        DesktopEntryData browser;
        DesktopEntryData email;
        DesktopEntryData terminal;
        QVERIFY(DesktopEntry::parse(
                "browser", "/applications/browser.desktop",
                "[Desktop Entry]\nType=Application\nName=Browser\nExec=browser %U\n"
                "Categories=WebBrowser;\nMimeType=x-scheme-handler/http;\n", &browser));
        QVERIFY(DesktopEntry::parse(
                "email", "/applications/email.desktop",
                "[Desktop Entry]\nType=Application\nName=Email\nExec=email %U\n"
                "Categories=Email;\nMimeType=x-scheme-handler/mailto;\n", &email));
        QVERIFY(DesktopEntry::parse(
                "terminal", "/applications/terminal.desktop",
                "[Desktop Entry]\nType=Application\nName=Terminal\nExec=terminal\n"
                "Categories=TerminalEmulator;\n", &terminal));
        registry->applyScan({browser, email, terminal});
        DefaultApplications settings;
        QCOMPARE(settings.browserList().size(), 1);
        QCOMPARE(settings.emailList().size(), 1);
        QCOMPARE(settings.terminalList().size(), 1);
        settings.setDefaultBrowser(0);
        settings.setDefaultEMail(0);
        settings.setDefaultTerminal(0);
        QCOMPARE(settings.browserIndex(), 0);
        QCOMPARE(settings.emailIndex(), 0);
        QCOMPARE(MimeApps::associations(path, "Default Applications", "x-scheme-handler/http"),
                 QStringList {"browser.desktop"});
        QCOMPARE(MimeApps::associations(path, "Default Applications", "x-scheme-handler/https"),
                 QStringList {"browser.desktop"});
        QCOMPARE(MimeApps::associations(path, "Default Applications", "x-scheme-handler/mailto"),
                 QStringList {"email.desktop"});
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().contains("x-scheme-handler/http=browser.desktop;"));
        QCOMPARE(MimeApps::defaultTerminal(dir.filePath("xdg-terminals.list")),
                 QStringLiteral("terminal.desktop"));
        DefaultApplications reopened;
        QCOMPARE(reopened.browserIndex(), 0);
        QCOMPARE(reopened.emailIndex(), 0);
        QCOMPARE(reopened.terminalIndex(), 0);
    }

    void preservesAssociations()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath("mimeapps.list");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("# Keep this comment\n[Default Applications]\nimage/png=image.desktop;\n"
                   "[Added Associations]\ntext/plain=old.desktop;new.desktop;\n"
                   "[Removed Associations]\ntext/plain=new.desktop;blocked.desktop;\n");
        file.close();
        QVERIFY(MimeApps::setDefault(path, "text/plain", "new.desktop"));
        QVERIFY(MimeApps::setDefault(path, "text/plain", "new.desktop"));
        QCOMPARE(MimeApps::associations(path, "Default Applications", "text/plain"),
                 QStringList {"new.desktop"});
        QCOMPARE(MimeApps::associations(path, "Default Applications", "image/png"),
                 QStringList {"image.desktop"});
        QCOMPARE(MimeApps::associations(path, "Added Associations", "text/plain"),
                 QStringList({"new.desktop", "old.desktop"}));
        QCOMPARE(MimeApps::associations(path, "Removed Associations", "text/plain"),
                 QStringList {"blocked.desktop"});
        QVERIFY(file.open(QIODevice::ReadOnly));
        const auto contents = file.readAll();
        QVERIFY(contents.contains("# Keep this comment"));
        QVERIFY(contents.contains("text/plain=new.desktop;"));
        QVERIFY(!contents.contains("text%2Fplain"));
        QVERIFY(!MimeApps::setDefault(path, "text/plain", "bad;entry.desktop"));
        QVERIFY(!MimeApps::setDefault(dir.path(), "text/plain", "new.desktop"));
    }

    void terminalListPrependsTheDefault()
    {
        QTemporaryDir dir;
        const QString path = dir.filePath("xdg-terminals.list");
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("# Keep this comment\n-blocked.desktop\nfirst.desktop\n+new.desktop:new-window\n");
        file.close();
        QCOMPARE(MimeApps::defaultTerminal(path), QStringLiteral("first.desktop"));
        QVERIFY(MimeApps::setDefaultTerminal(path, "new.desktop"));
        QCOMPARE(MimeApps::defaultTerminal(path), QStringLiteral("new.desktop"));
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), QByteArray("new.desktop\n# Keep this comment\n-blocked.desktop\n"
                                            "first.desktop\n"));
        QVERIFY(!MimeApps::setDefaultTerminal(path, "bad\nentry.desktop"));
    }
};
QTEST_GUILESS_MAIN(MimeAppsTest)
#include "tst_mimeapps.moc"
