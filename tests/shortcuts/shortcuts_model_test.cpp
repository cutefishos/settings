// ShortcutsModel against a stand-in for the compositor's com.cutefish.Shortcuts1
// service, answering in the JSON the real one produces.

#include "shortcuts/shortcutsmodel.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSignalSpy>
#include <QTest>

static QJsonObject accelerator(int keysym, const QString &keyName, const QString &keyText, int modifiers,
                               const QString &type = QStringLiteral("default"))
{
    return {{QStringLiteral("keysym"), keysym},
            {QStringLiteral("keyName"), keyName},
            {QStringLiteral("keyText"), keyText},
            {QStringLiteral("modifiers"), modifiers},
            {QStringLiteral("keyState"), QStringLiteral("pressed")},
            {QStringLiteral("type"), type},
            {QStringLiteral("locked"), false}};
}

namespace {

class FakeShortcutsService : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.cutefish.Shortcuts1")
    Q_PROPERTY(QString shortcutConvention MEMBER convention)

public:
    QString convention = QStringLiteral("pc");
    QJsonArray maximize = {accelerator(0x3d, QStringLiteral("equal"), QStringLiteral("="), 4),
                           accelerator(0xff52, QStringLiteral("Up"), QString(), 8)};
    QJsonArray maximizeDefaults = maximize;
    QJsonArray lock = {accelerator(0x6c, QStringLiteral("l"), QStringLiteral("L"), 8)};
    QStringList calls;
    QString nextResult = QStringLiteral("{\"result\":\"success\"}");

public slots:
    QString accelerators()
    {
        QJsonArray actions;
        actions.append(QJsonObject {{QStringLiteral("name"), QStringLiteral("lock-screen")},
                                    {QStringLiteral("accelerators"), lock},
                                    {QStringLiteral("defaults"), lock}});
        actions.append(QJsonObject {{QStringLiteral("name"), QStringLiteral("toggle-maximized")},
                                    {QStringLiteral("accelerators"), maximize},
                                    {QStringLiteral("defaults"), maximizeDefaults}});
        actions.append(QJsonObject {{QStringLiteral("name"), QStringLiteral("not-a-cutefish-action")},
                                    {QStringLiteral("accelerators"), QJsonArray()},
                                    {QStringLiteral("defaults"), QJsonArray()}});
        return QString::fromUtf8(
                QJsonDocument(QJsonObject {{QStringLiteral("actions"), actions}}).toJson());
    }

    QString addAccelerator(const QString &action, uint keycode, uint keysym, uint modifiers)
    {
        calls << QStringLiteral("add %1 %2 %3 %4").arg(action).arg(keycode).arg(keysym).arg(modifiers);
        return nextResult;
    }

    QString removeAccelerator(const QString &action, uint keysym, uint modifiers)
    {
        calls << QStringLiteral("remove %1 %2 %3").arg(action).arg(keysym).arg(modifiers);
        return nextResult;
    }

    QString replaceAccelerator(const QString &action, uint oldKeysym, uint oldModifiers,
                               uint keycode, uint keysym, uint modifiers)
    {
        calls << QStringLiteral("replace %1 %2 %3 %4 %5 %6")
                         .arg(action).arg(oldKeysym).arg(oldModifiers).arg(keycode).arg(keysym)
                         .arg(modifiers);
        return nextResult;
    }

    QString restoreDefault(const QString &action)
    {
        calls << QStringLiteral("restore %1").arg(action);
        return nextResult;
    }

    QString restoreAllDefaults()
    {
        calls << QStringLiteral("restore-all");
        return nextResult;
    }

    QString describeKey(uint keycode, uint keysym)
    {
        calls << QStringLiteral("describe %1 %2").arg(keycode).arg(keysym);
        return QStringLiteral("{\"keysym\":113,\"keyName\":\"q\",\"keyText\":\"Q\"}");
    }

    void preventProcessingAccelerators(bool prevent)
    {
        calls << QStringLiteral("prevent %1").arg(prevent);
    }

    bool hasCustomAccelerators() { return maximize != maximizeDefaults; }

signals:
    void acceleratorsChanged();
};

} // namespace

class ShortcutsModelTest : public QObject
{
    Q_OBJECT

private slots:
    void listsKnownActionsInLayoutOrderWithReadableKeys()
    {
        FakeShortcutsService service;
        auto bus = QDBusConnection::sessionBus();
        // QtDBus caches an interface's introspection per connection, so every
        // test exports the same members.
        QVERIFY(bus.registerObject(QStringLiteral("/com/cutefish/Shortcuts1"), &service,
                                   QDBusConnection::ExportAllSlots
                                           | QDBusConnection::ExportAllSignals
                                           | QDBusConnection::ExportAllProperties));
        QVERIFY(bus.registerService(QStringLiteral("com.cutefish.Shortcuts1")));

        ShortcutsModel model;
        QTRY_VERIFY(model.available());
        QTRY_COMPARE(model.rowCount(), 2);
        // Layout order, not the service's order: lock-screen is in General
        // after the launcher entries, toggle-maximized is in Windows.
        QCOMPARE(model.data(model.index(0, 0), ShortcutsModel::NameRole).toString(),
                 QStringLiteral("lock-screen"));
        QCOMPARE(model.data(model.index(1, 0), ShortcutsModel::NameRole).toString(),
                 QStringLiteral("toggle-maximized"));
        QCOMPARE(model.data(model.index(1, 0), ShortcutsModel::CategoryRole).toString(),
                 QStringLiteral("Windows"));
        QVERIFY(model.data(model.index(1, 0), ShortcutsModel::IsDefaultRole).toBool());

        const QVariantList accelerators =
                model.data(model.index(1, 0), ShortcutsModel::AcceleratorsRole).toList();
        QCOMPARE(accelerators.size(), 2);
        QCOMPARE(accelerators.at(0).toMap().value(QStringLiteral("parts")).toStringList(),
                 QStringList({QStringLiteral("Alt"), QStringLiteral("=")}));
        QCOMPARE(accelerators.at(1).toMap().value(QStringLiteral("parts")).toStringList(),
                 QStringList({QStringLiteral("Meta"), QStringLiteral("↑")}));
        QCOMPARE(accelerators.at(1).toMap().value(QStringLiteral("text")).toString(),
                 QStringLiteral("Meta + ↑"));
        QVERIFY(!model.hasCustomAccelerators());

        // A change on the service side reloads the model.
        service.maximize = {accelerator(0x6b, QStringLiteral("k"), QStringLiteral("K"), 8,
                                        QStringLiteral("user"))};
        emit service.acceleratorsChanged();
        QTRY_VERIFY(model.hasCustomAccelerators());
        QVERIFY(!model.data(model.index(1, 0), ShortcutsModel::IsDefaultRole).toBool());

        // Requests reach the service with the compositor's modifier bits, and
        // results come back as messages the dialog shows.
        QVariantMap result = model.addAccelerator(QStringLiteral("toggle-maximized"), 16, 0,
                                                  ShortcutsModel::MetaDown);
        QCOMPARE(result.value(QStringLiteral("result")).toString(), QStringLiteral("success"));
        QVERIFY(result.value(QStringLiteral("message")).toString().isEmpty());
        service.nextResult = QStringLiteral(
                "{\"result\":\"conflict-can-override\",\"conflictAction\":\"lock-screen\"}");
        result = model.replaceAccelerator(QStringLiteral("toggle-maximized"), 0x6b, 8, 38, 0, 8);
        QCOMPARE(result.value(QStringLiteral("result")).toString(),
                 QStringLiteral("conflict-can-override"));
        QCOMPARE(result.value(QStringLiteral("conflictDescription")).toString(),
                 QStringLiteral("Lock the screen"));
        QVERIFY(result.value(QStringLiteral("message")).toString().contains(
                QStringLiteral("Lock the screen")));
        model.removeAccelerator(QStringLiteral("toggle-maximized"), 0x6b, 8);
        model.restoreDefault(QStringLiteral("toggle-maximized"));
        model.restoreAllDefaults();
        model.setRecording(true);
        QCOMPARE(model.partsForKey(16, Qt::Key_Q, ShortcutsModel::ControlDown
                                                          | ShortcutsModel::ShiftDown),
                 QStringList({QStringLiteral("Ctrl"), QStringLiteral("Shift"),
                              QStringLiteral("Q")}));
        model.setRecording(false);
        QTRY_COMPARE(service.calls.size(), 8);
        QCOMPARE(service.calls.at(0), QStringLiteral("add toggle-maximized 16 0 8"));
        QCOMPARE(service.calls.at(1), QStringLiteral("replace toggle-maximized 107 8 38 0 8"));
        QCOMPARE(service.calls.at(2), QStringLiteral("remove toggle-maximized 107 8"));
        QCOMPARE(service.calls.at(3), QStringLiteral("restore toggle-maximized"));
        QCOMPARE(service.calls.at(4), QStringLiteral("restore-all"));
        QCOMPARE(service.calls.at(5), QStringLiteral("prevent 1"));
        QCOMPARE(service.calls.at(6), QStringLiteral("describe 16 81"));
        QCOMPARE(service.calls.at(7), QStringLiteral("prevent 0"));

        QCOMPARE(model.compositorModifiers(Qt::ControlModifier | Qt::MetaModifier),
                 int(ShortcutsModel::ControlDown | ShortcutsModel::MetaDown));
        QCOMPARE(model.keysymForQtKey(Qt::Key_F4), 0xffc1);
        QCOMPARE(model.keysymForQtKey(Qt::Key_Up), 0xff52);
        QVERIFY(model.isModifierKey(Qt::Key_Meta));
        QVERIFY(!model.isModifierKey(Qt::Key_Q));

        bus.unregisterObject(QStringLiteral("/com/cutefish/Shortcuts1"));
        bus.unregisterService(QStringLiteral("com.cutefish.Shortcuts1"));
        QTRY_VERIFY(!model.available());
        QCOMPARE(model.rowCount(), 0);
    }

    void theCommandConventionNamesTheKeys()
    {
        FakeShortcutsService service;
        service.convention = QStringLiteral("command");
        auto bus = QDBusConnection::sessionBus();
        QVERIFY(bus.registerObject(QStringLiteral("/com/cutefish/Shortcuts1"), &service,
                                   QDBusConnection::ExportAllSlots
                                           | QDBusConnection::ExportAllSignals
                                           | QDBusConnection::ExportAllProperties));
        QVERIFY(bus.registerService(QStringLiteral("com.cutefish.Shortcuts1")));

        ShortcutsModel model;
        QTRY_VERIFY(model.available());
        QTRY_VERIFY(model.commandKeys());
        QCOMPARE(model.modifierParts(ShortcutsModel::ControlDown | ShortcutsModel::AltDown
                                     | ShortcutsModel::MetaDown),
                 QStringList({QStringLiteral("Control"), QStringLiteral("Option"),
                              QStringLiteral("Command")}));

        bus.unregisterObject(QStringLiteral("/com/cutefish/Shortcuts1"));
        bus.unregisterService(QStringLiteral("com.cutefish.Shortcuts1"));
    }

    void reportsTheServiceMissing()
    {
        ShortcutsModel model;
        QVERIFY(!model.available());
        QCOMPARE(model.rowCount(), 0);
        const QVariantMap result = model.restoreAllDefaults();
        QCOMPARE(result.value(QStringLiteral("result")).toString(), QStringLiteral("unavailable"));
        QVERIFY(!result.value(QStringLiteral("message")).toString().isEmpty());
    }
};

int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    ShortcutsModelTest test;
    return QTest::qExec(&test, argc, argv);
}

#include "shortcuts_model_test.moc"
