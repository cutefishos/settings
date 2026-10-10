#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include <QAbstractListModel>
#include <QDBusInterface>
#include <QJsonObject>
#include <QVariantMap>
#include <QVector>

// The keyboard shortcuts page's view of the compositor's com.cutefish.Shortcuts1
// service. The compositor owns the bindings and the defaults; this side owns
// the order the actions are listed in, their categories and their
// descriptions, the way ash/webui/shortcut_customization_ui's layout table
// does for the ChromeOS Shortcuts app.
class ShortcutsModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool hasCustomAccelerators READ hasCustomAccelerators NOTIFY acceleratorsChanged)
    Q_PROPERTY(QStringList categories READ categories CONSTANT)
    // Command is the shortcut key: the modifiers go by their Command names.
    Q_PROPERTY(bool commandKeys READ commandKeys NOTIFY acceleratorsChanged)

public:
    enum Role {
        NameRole = Qt::UserRole + 1,
        DescriptionRole,
        CategoryRole,
        AcceleratorsRole,
        IsDefaultRole,
    };
    Q_ENUM(Role)

    // The compositor's modifier bits.
    enum Modifier {
        ShiftDown = 1 << 0,
        ControlDown = 1 << 1,
        AltDown = 1 << 2,
        MetaDown = 1 << 3,
    };
    Q_ENUM(Modifier)

    explicit ShortcutsModel(QObject *parent = nullptr);
    ~ShortcutsModel() override;

    bool available() const { return m_available; }
    bool hasCustomAccelerators() const { return m_hasCustomAccelerators; }
    QStringList categories() const;
    bool commandKeys() const { return m_commandKeys; }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    // Each answers {"result": <name>, "message": <text for the user>,
    // "conflictAction": <name>, "conflictDescription": <text>}; "success"
    // and "restore-success-with-conflicts" mean the change was made.
    Q_INVOKABLE QVariantMap addAccelerator(const QString &action, int keycode, int keysym,
                                           int modifiers);
    Q_INVOKABLE QVariantMap replaceAccelerator(const QString &action, int oldKeysym,
                                               int oldModifiers, int keycode, int keysym,
                                               int modifiers);
    Q_INVOKABLE QVariantMap removeAccelerator(const QString &action, int keysym, int modifiers);
    Q_INVOKABLE QVariantMap restoreDefault(const QString &action);
    Q_INVOKABLE QVariantMap restoreAllDefaults();

    // While recording, the compositor performs no shortcut, so pressing Meta
    // does not open the launcher under the dialog.
    Q_INVOKABLE void setRecording(bool recording);

    // The parts of a shortcut as the user reads them: ["Ctrl", "Shift", "Q"].
    Q_INVOKABLE QStringList modifierParts(int modifiers) const;
    Q_INVOKABLE QStringList partsForKey(int keycode, int qtKey, int modifiers);
    Q_INVOKABLE bool isModifierKey(int qtKey) const;
    Q_INVOKABLE int compositorModifiers(int qtModifiers) const;
    Q_INVOKABLE int keysymForQtKey(int qtKey) const;
    Q_INVOKABLE QString descriptionFor(const QString &action) const;

    // {"description", "accelerators", "isDefault"} for one action, for the
    // edit dialog; empty when the action is unknown.
    Q_INVOKABLE QVariantMap action(const QString &name) const;

signals:
    void availableChanged();
    void acceleratorsChanged();

private slots:
    void reload();

private:
    struct Layout {
        const char *name;
        QString category;
        QString description;
    };

    struct Action {
        QString name;
        QString category;
        QString description;
        QVariantList accelerators;
        QVariantList defaults;
    };

    void refreshInterface();
    QVariantMap acceleratorFromJson(const QJsonObject &object) const;
    QString keyDisplay(const QString &keyName, const QString &keyText) const;
    QVariantMap call(const QString &method, const QVariantList &arguments);
    QVariantMap resultFromJson(const QString &json) const;
    QString messageFor(const QString &result, const QString &conflictDescription) const;

    QDBusInterface *m_interface = nullptr;
    QVector<Action> m_actions;
    bool m_available = false;
    bool m_hasCustomAccelerators = false;
    bool m_commandKeys = false;
};

#endif // SHORTCUTSMODEL_H
