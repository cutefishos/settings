#include "language.h"

#include <QAbstractItemModelTester>
#include <QElapsedTimer>
#include <QSettings>
#include <QTemporaryDir>
#include <QTest>

class LanguageTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        QVERIFY(m_config.isValid());
        qputenv("XDG_CONFIG_HOME", m_config.path().toUtf8());
        QSettings settings("cutefish", "language");
        settings.setValue("language", "en_US");
        settings.setValue("preferredLanguages", QStringList{"en_US", "zh_Hans_CN"});
    }

    void pageDoesNotLoadCatalog()
    {
        QElapsedTimer timer;
        timer.start();
        Language language;
        QCOMPARE(language.preferredLanguages().size(), 2);
        QCOMPARE(language.availableLanguages()->rowCount(), 0);
        QVERIFY(!language.loading());
        qInfo() << "Page backend construction:" << timer.nsecsElapsed() / 1000000.0 << "ms";
    }

    void catalogAndSearch()
    {
        Language language;
        auto *model = language.availableLanguages();
        QAbstractItemModelTester tester(model, QAbstractItemModelTester::FailureReportingMode::QtTest);
        language.loadLanguages();
        QVERIFY(language.loading());
        QTRY_VERIFY_WITH_TIMEOUT(!language.loading(), 5000);
        QCOMPARE(model->rowCount(), 40);
        QCOMPARE(model->data(model->index(0, 0), LanguageModel::CodeRole).toString(), "en_GB");
        language.search("zh_Hans_CN");
        QCOMPARE(model->rowCount(), 0);
        language.search("");

        QElapsedTimer timer;
        timer.start();
        language.search("中文");
        QVERIFY(model->rowCount() > 0);
        language.search("it_IT");
        QCOMPARE(model->rowCount(), 1);
        language.search("no-such-language");
        QCOMPARE(model->rowCount(), 0);
        language.search("  ID_id  ");
        QCOMPARE(model->rowCount(), 1);
        qInfo() << "Four catalog searches:" << timer.nsecsElapsed() / 1000000.0 << "ms";
        language.search("");
        QCOMPARE(model->rowCount(), 40);
    }

    void preferredLanguagesAreFilteredDynamically()
    {
        LanguageModel source;
        AvailableLanguageModel model(&source);
        source.setEntries({{"en_US", "English", "English", "en_US", true},
                           {"ja_JP", "日本語", "Japanese", "ja_JP", true}});
        QCOMPARE(model.rowCount(), 2);
        source.setPreferredCodes({"en_US"});
        QCOMPARE(model.rowCount(), 1);
        QCOMPARE(model.data(model.index(0, 0), LanguageModel::CodeRole).toString(), "ja_JP");
        model.setFilterFixedString("en_US");
        QCOMPARE(model.rowCount(), 0);
        source.setPreferredCodes({});
        QCOMPARE(model.rowCount(), 1);
    }

    void closingWhileLoading()
    {
        auto *language = new Language;
        language->loadLanguages();
        delete language;
        Language reopened;
        reopened.loadLanguages();
        QTRY_VERIFY_WITH_TIMEOUT(!reopened.loading(), 5000);
        QCOMPARE(reopened.availableLanguages()->rowCount(), 40);
    }

private:
    QTemporaryDir m_config;
};

QTEST_GUILESS_MAIN(LanguageTest)
#include "language_test.moc"
