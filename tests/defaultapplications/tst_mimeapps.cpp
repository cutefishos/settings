#include "mimeapps.h"
#include "defaultapplications.h"
#include "applicationregistry.h"
#include <QDir>
#include <QFile>
#include <QMimeDatabase>
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
        // Apart, so saving a list does not make the registry rescan its folders.
        const QString config = dir.filePath("config");
        const QString data = dir.filePath("data");
        QVERIFY(QDir().mkpath(config));
        QVERIFY(QDir().mkpath(data));
        qputenv("XDG_CONFIG_HOME", config.toUtf8());
        qputenv("XDG_DATA_HOME", data.toUtf8());
        qputenv("XDG_DATA_DIRS", data.toUtf8());
        // The system MIME database, without the system's applications.
        QVERIFY(QFile::link(QStringLiteral("/usr/share/mime"), data + "/mime"));
        const QString path = config + "/mimeapps.list";
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
        DesktopEntryData player;
        DesktopEntryData otherPlayer;
        DesktopEntryData editor;
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
        QVERIFY(DesktopEntry::parse(
                "player", "/applications/player.desktop",
                "[Desktop Entry]\nType=Application\nName=Player\nExec=player %U\n"
                "Categories=AudioVideo;Video;Player;\n"
                "MimeType=video/mp4;video/webm;application/ogg;\n", &player));
        QVERIFY(DesktopEntry::parse(
                "other-player", "/applications/other-player.desktop",
                "[Desktop Entry]\nType=Application\nName=Other Player\nExec=other %U\n"
                "Categories=AudioVideo;\nMimeType=video/mp4;\n", &otherPlayer));
        QVERIFY(DesktopEntry::parse(
                "editor", "/applications/editor.desktop",
                "[Desktop Entry]\nType=Application\nName=Editor\nExec=editor %U\n"
                "Categories=Graphics;\nMimeType=video/mp4;\n", &editor));
        registry->applyScan({browser, email, terminal, player, otherPlayer, editor});
        DefaultApplications settings;
        QCOMPARE(settings.browserList().size(), 1);
        QCOMPARE(settings.emailList().size(), 1);
        QCOMPARE(settings.terminalList().size(), 1);
        QCOMPARE(settings.videoPlayerList().size(), 2);
        // Without a choice, the player the file manager would open videos with.
        const auto videoPlayerIndex = [&settings](const QString &path) {
            const QVariantList players = settings.videoPlayerList();
            for (int i = 0; i < players.size(); ++i) {
                if (players.at(i).toMap().value("path").toString() == path)
                    return i;
            }
            return -1;
        };
        const QMimeType mp4 = QMimeDatabase().mimeTypeForName("video/mp4");
        QCOMPARE(settings.videoPlayerIndex(), videoPlayerIndex(MimeApps::applications(mp4).value(0)));
        const int chosen = videoPlayerIndex("/applications/player.desktop");
        const int other = videoPlayerIndex("/applications/other-player.desktop");
        settings.setDefaultBrowser(0);
        settings.setDefaultEMail(0);
        settings.setDefaultTerminal(0);
        settings.setDefaultVideoPlayer(chosen);
        QCOMPARE(settings.browserIndex(), 0);
        QCOMPARE(settings.emailIndex(), 0);
        QCOMPARE(MimeApps::associations(path, "Default Applications", "x-scheme-handler/http"),
                 QStringList {"browser.desktop"});
        QCOMPARE(MimeApps::associations(path, "Default Applications", "x-scheme-handler/https"),
                 QStringList {"browser.desktop"});
        QCOMPARE(MimeApps::associations(path, "Default Applications", "x-scheme-handler/mailto"),
                 QStringList {"email.desktop"});
        QCOMPARE(MimeApps::associations(path, "Default Applications", "video/mp4"),
                 QStringList {"player.desktop"});
        QCOMPARE(MimeApps::associations(path, "Default Applications", "video/webm"),
                 QStringList {"player.desktop"});
        QVERIFY(MimeApps::associations(path, "Default Applications", "application/ogg").isEmpty());
        QFile file(path);
        QVERIFY(file.open(QIODevice::ReadOnly));
        QVERIFY(file.readAll().contains("x-scheme-handler/http=browser.desktop;"));
        QCOMPARE(MimeApps::defaultTerminal(config + "/xdg-terminals.list"),
                 QStringLiteral("terminal.desktop"));
        DefaultApplications reopened;
        QCOMPARE(reopened.browserIndex(), 0);
        QCOMPARE(reopened.emailIndex(), 0);
        QCOMPARE(reopened.terminalIndex(), 0);
        QCOMPARE(reopened.videoPlayerIndex(), chosen);

        // A choice made elsewhere shows without reopening.
        QVERIFY(MimeApps::setDefault(path, "video/mp4", "other-player.desktop"));
        QTRY_COMPARE(reopened.videoPlayerIndex(), other);
    }
};
QTEST_GUILESS_MAIN(MimeAppsTest)
#include "tst_mimeapps.moc"
