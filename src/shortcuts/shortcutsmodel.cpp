#include "shortcutsmodel.h"

#include <QDBusReply>
#include <QKeySequence>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>

namespace {
constexpr auto service = "com.cutefish.Shortcuts1";
constexpr auto path = "/com/cutefish/Shortcuts1";

// Qt's key codes for the keys that are not printable characters, and the xkb
// keysyms the compositor knows them by. Only used when the toolkit reports no
// scan code; the compositor resolves the scan code itself otherwise.
struct QtKeysym {
    int qtKey;
    int keysym;
};

const QtKeysym kQtKeysyms[] = {
        {Qt::Key_Escape, 0xff1b},   {Qt::Key_Tab, 0xff09},      {Qt::Key_Backtab, 0xfe20},
        {Qt::Key_Backspace, 0xff08}, {Qt::Key_Return, 0xff0d},  {Qt::Key_Enter, 0xff8d},
        {Qt::Key_Insert, 0xff63},   {Qt::Key_Delete, 0xffff},   {Qt::Key_Pause, 0xff13},
        {Qt::Key_Print, 0xff61},    {Qt::Key_Home, 0xff50},     {Qt::Key_End, 0xff57},
        {Qt::Key_Left, 0xff51},     {Qt::Key_Up, 0xff52},       {Qt::Key_Right, 0xff53},
        {Qt::Key_Down, 0xff54},     {Qt::Key_PageUp, 0xff55},   {Qt::Key_PageDown, 0xff56},
        {Qt::Key_Space, 0x20},      {Qt::Key_Menu, 0xff67},     {Qt::Key_Meta, 0xffeb},
        {Qt::Key_Super_L, 0xffeb},  {Qt::Key_Super_R, 0xffec},
        {Qt::Key_VolumeDown, 0x1008ff11}, {Qt::Key_VolumeMute, 0x1008ff12},
        {Qt::Key_VolumeUp, 0x1008ff13}, {Qt::Key_MediaPlay, 0x1008ff14},
        {Qt::Key_MediaStop, 0x1008ff15}, {Qt::Key_MediaPrevious, 0x1008ff16},
        {Qt::Key_MediaNext, 0x1008ff17}, {Qt::Key_MonBrightnessUp, 0x1008ff02},
        {Qt::Key_MonBrightnessDown, 0x1008ff03}, {Qt::Key_Calculator, 0x1008ff1d},
};
} // namespace

ShortcutsModel::ShortcutsModel(QObject *parent)
    : QAbstractListModel(parent)
{
    auto *watcher = new QDBusServiceWatcher(QLatin1String(service), QDBusConnection::sessionBus(),
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this,
            &ShortcutsModel::refreshInterface);
    refreshInterface();
}

ShortcutsModel::~ShortcutsModel()
{
    setRecording(false);
}

QStringList ShortcutsModel::categories() const
{
    return {tr("General"), tr("Windows")};
}

void ShortcutsModel::refreshInterface()
{
    delete m_interface;
    m_interface = new QDBusInterface(QLatin1String(service), QLatin1String(path),
                                     QLatin1String(service), QDBusConnection::sessionBus(), this);
    const bool available = m_interface->isValid();
    if (available)
        connect(m_interface, SIGNAL(acceleratorsChanged()), this, SLOT(reload()));
    if (available != m_available) {
        m_available = available;
        emit availableChanged();
    }
    reload();
}

void ShortcutsModel::reload()
{
    // The order here is the order on the page, like kAcceleratorLayouts.
    const QString general = tr("General");
    const QString windows = tr("Windows");
    const Layout layouts[] = {
            {"toggle-app-list", general, tr("Open the launcher")},
            {"lock-screen", general, tr("Lock the screen")},

            {"cycle-forward-mru", windows, tr("Switch to the next window")},
            {"cycle-backward-mru", windows, tr("Switch to the previous window")},
            {"toggle-maximized", windows, tr("Maximize or restore the window")},
            {"window-minimize", windows, tr("Minimize the window")},
            {"quit-application", windows, tr("Quit the application")},
            {"hide-application", windows, tr("Hide the application")},
            {"hide-other-applications", windows, tr("Hide other applications")},
            {"toggle-fullscreen", windows, tr("Toggle full screen")},
            {"window-cycle-snap-left", windows, tr("Snap the window to the left")},
            {"window-cycle-snap-right", windows, tr("Snap the window to the right")},
            {"close-window", windows, tr("Close the window")},
            {"move-active-window-between-displays", windows, tr("Move the window to the next display")},
    };

    QHash<QString, QJsonObject> published;
    bool hasCustom = false;
    if (m_interface && m_interface->isValid()) {
        const QDBusReply<QString> reply = m_interface->call(QStringLiteral("accelerators"));
        if (reply.isValid()) {
            const QJsonArray actions = QJsonDocument::fromJson(reply.value().toUtf8())
                                               .object()
                                               .value(QStringLiteral("actions"))
                                               .toArray();
            for (const QJsonValue &value : actions) {
                const QJsonObject action = value.toObject();
                published.insert(action.value(QStringLiteral("name")).toString(), action);
            }
        }
        const QDBusReply<bool> custom = m_interface->call(QStringLiteral("hasCustomAccelerators"));
        hasCustom = custom.isValid() && custom.value();
        m_commandKeys = m_interface->property("shortcutConvention").toString()
                == QLatin1String("command");
        // A compositor that does not say is one whose shortcuts are all editable.
        const QVariant customizable = m_interface->property("customizable");
        m_customizable = !customizable.isValid() || customizable.toBool();
    }

    beginResetModel();
    m_actions.clear();
    for (const Layout &layout : layouts) {
        const auto it = published.constFind(QLatin1String(layout.name));
        // An action the compositor does not publish is one it cannot perform.
        if (it == published.constEnd())
            continue;
        Action action;
        action.name = QLatin1String(layout.name);
        action.category = layout.category;
        action.description = layout.description;
        for (const QJsonValue &value : it->value(QStringLiteral("accelerators")).toArray())
            action.accelerators.append(acceleratorFromJson(value.toObject()));
        for (const QJsonValue &value : it->value(QStringLiteral("defaults")).toArray())
            action.defaults.append(acceleratorFromJson(value.toObject()));
        m_actions.append(action);
    }
    endResetModel();
    m_hasCustomAccelerators = hasCustom;
    emit acceleratorsChanged();
}

QVariantMap ShortcutsModel::acceleratorFromJson(const QJsonObject &object) const
{
    QVariantMap accelerator;
    const int modifiers = object.value(QStringLiteral("modifiers")).toInt();
    const QString keyName = object.value(QStringLiteral("keyName")).toString();
    const QString keyText = object.value(QStringLiteral("keyText")).toString();
    accelerator.insert(QStringLiteral("keysym"), object.value(QStringLiteral("keysym")).toInt());
    accelerator.insert(QStringLiteral("modifiers"), modifiers);
    accelerator.insert(QStringLiteral("keyName"), keyName);
    accelerator.insert(QStringLiteral("type"), object.value(QStringLiteral("type")).toString());
    accelerator.insert(QStringLiteral("locked"), object.value(QStringLiteral("locked")).toBool());
    QStringList parts = modifierParts(modifiers);
    parts.append(keyDisplay(keyName, keyText));
    accelerator.insert(QStringLiteral("parts"), parts);
    accelerator.insert(QStringLiteral("text"), parts.join(QStringLiteral(" + ")));
    return accelerator;
}

QString ShortcutsModel::keyDisplay(const QString &keyName, const QString &keyText) const
{
    if (keyName == QLatin1String("Super_L") || keyName == QLatin1String("Super_R"))
        return m_commandKeys ? tr("Command") : tr("Meta");
    static const QHash<QString, QString> names = {
            {QStringLiteral("Up"), QStringLiteral("↑")},
            {QStringLiteral("Down"), QStringLiteral("↓")},
            {QStringLiteral("Left"), QStringLiteral("←")},
            {QStringLiteral("Right"), QStringLiteral("→")},
            {QStringLiteral("Escape"), tr("Esc")},
            {QStringLiteral("Return"), tr("Enter")},
            {QStringLiteral("KP_Enter"), tr("Enter")},
            {QStringLiteral("space"), tr("Space")},
            {QStringLiteral("BackSpace"), tr("Backspace")},
            {QStringLiteral("Delete"), tr("Delete")},
            {QStringLiteral("Insert"), tr("Insert")},
            {QStringLiteral("Home"), tr("Home")},
            {QStringLiteral("End"), tr("End")},
            {QStringLiteral("Prior"), tr("Page Up")},
            {QStringLiteral("Next"), tr("Page Down")},
            {QStringLiteral("Print"), tr("Print Screen")},
            {QStringLiteral("Tab"), tr("Tab")},
            {QStringLiteral("Menu"), tr("Menu")},
            {QStringLiteral("XF86AudioRaiseVolume"), tr("Volume Up")},
            {QStringLiteral("XF86AudioLowerVolume"), tr("Volume Down")},
            {QStringLiteral("XF86AudioMute"), tr("Mute")},
            {QStringLiteral("XF86MonBrightnessUp"), tr("Brightness Up")},
            {QStringLiteral("XF86MonBrightnessDown"), tr("Brightness Down")},
            {QStringLiteral("XF86AudioPlay"), tr("Play/Pause")},
            {QStringLiteral("XF86AudioPause"), tr("Pause")},
            {QStringLiteral("XF86AudioNext"), tr("Next Track")},
            {QStringLiteral("XF86AudioPrev"), tr("Previous Track")},
            {QStringLiteral("XF86AudioStop"), tr("Stop")},
            {QStringLiteral("XF86Calculator"), tr("Calculator")},
            {QStringLiteral("XF86Explorer"), tr("File Manager")},
            {QStringLiteral("XF86Search"), tr("Search")},
            {QStringLiteral("XF86HomePage"), tr("Home Page")},
            {QStringLiteral("XF86Mail"), tr("Mail")},
    };
    if (!keyText.isEmpty())
        return keyText;
    const auto it = names.constFind(keyName);
    if (it != names.constEnd())
        return it.value();
    return keyName;
}

int ShortcutsModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_actions.size();
}

QVariant ShortcutsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_actions.size())
        return QVariant();
    const Action &action = m_actions.at(index.row());
    switch (role) {
    case NameRole:
        return action.name;
    case DescriptionRole:
        return action.description;
    case CategoryRole:
        return action.category;
    case AcceleratorsRole:
        return action.accelerators;
    case IsDefaultRole:
        return action.accelerators == action.defaults;
    case LockedRole:
        return action.locked;
    }
    return QVariant();
}

QHash<int, QByteArray> ShortcutsModel::roleNames() const
{
    return {
            {NameRole, "name"},
            {DescriptionRole, "description"},
            {CategoryRole, "category"},
            {AcceleratorsRole, "accelerators"},
            {IsDefaultRole, "isDefault"},
            {LockedRole, "locked"},
    };
}

QVariantMap ShortcutsModel::call(const QString &method, const QVariantList &arguments)
{
    if (!m_interface || !m_interface->isValid()) {
        return {{QStringLiteral("result"), QStringLiteral("unavailable")},
                {QStringLiteral("message"), tr("The shortcut service is not available.")}};
    }
    const QDBusReply<QString> reply = m_interface->callWithArgumentList(QDBus::Block, method,
                                                                         arguments);
    if (!reply.isValid()) {
        qWarning() << "Shortcut request" << method << "failed:" << reply.error().message();
        return {{QStringLiteral("result"), QStringLiteral("error")},
                {QStringLiteral("message"), reply.error().message()}};
    }
    return resultFromJson(reply.value());
}

QVariantMap ShortcutsModel::resultFromJson(const QString &json) const
{
    const QJsonObject object = QJsonDocument::fromJson(json.toUtf8()).object();
    const QString result = object.value(QStringLiteral("result")).toString();
    const QString conflictAction = object.value(QStringLiteral("conflictAction")).toString();
    const QString conflictDescription = descriptionFor(conflictAction);
    return {
            {QStringLiteral("result"), result},
            {QStringLiteral("conflictAction"), conflictAction},
            {QStringLiteral("conflictDescription"), conflictDescription},
            {QStringLiteral("message"), messageFor(result, conflictDescription)},
    };
}

QString ShortcutsModel::messageFor(const QString &result, const QString &conflictDescription) const
{
    if (result == QLatin1String("success"))
        return QString();
    if (result == QLatin1String("conflict"))
        return tr("This shortcut is already used by \"%1\".").arg(conflictDescription);
    if (result == QLatin1String("conflict-can-override"))
        return tr("This shortcut is used by \"%1\". Press it again to reassign it.")
                .arg(conflictDescription);
    if (result == QLatin1String("missing-modifier")) {
        return m_commandKeys
                ? tr("Include Control, Option or Command, or use a function key.")
                : tr("Include Ctrl, Alt or Meta, or use a function key.");
    }
    if (result == QLatin1String("shift-only-not-allowed")) {
        return m_commandKeys
                ? tr("Shift cannot be the only modifier. Add Control, Option or Command.")
                : tr("Shift cannot be the only modifier. Add Ctrl, Alt or Meta.");
    }
    if (result == QLatin1String("key-not-allowed") || result == QLatin1String("reserved-key-not-allowed"))
        return tr("This key cannot be used in a shortcut.");
    if (result == QLatin1String("maximum-accelerators-reached"))
        return tr("This action already has the maximum of five shortcuts.");
    if (result == QLatin1String("non-meta-accelerator-warning")) {
        return m_commandKeys ? tr("Shortcuts without Command may be intercepted before "
                                  "applications see them. Press it again to keep it.")
                             : tr("Shortcuts without Meta may be intercepted before applications "
                                  "see them. Press it again to keep it.");
    }
    if (result == QLatin1String("restore-success-with-conflicts"))
        return tr("Some default shortcuts are in use by other actions and were not restored.");
    if (result == QLatin1String("action-locked") || result == QLatin1String("accelerator-locked"))
        return tr("This shortcut cannot be changed.");
    if (result == QLatin1String("not-found"))
        return tr("This shortcut no longer exists.");
    return tr("The shortcut could not be changed.");
}

QVariantMap ShortcutsModel::addAccelerator(const QString &action, int keycode, int keysym,
                                           int modifiers)
{
    return call(QStringLiteral("addAccelerator"),
                {action, static_cast<uint>(qMax(0, keycode)), static_cast<uint>(qMax(0, keysym)),
                 static_cast<uint>(modifiers)});
}

QVariantMap ShortcutsModel::replaceAccelerator(const QString &action, int oldKeysym,
                                               int oldModifiers, int keycode, int keysym,
                                               int modifiers)
{
    return call(QStringLiteral("replaceAccelerator"),
                {action, static_cast<uint>(oldKeysym), static_cast<uint>(oldModifiers),
                 static_cast<uint>(qMax(0, keycode)), static_cast<uint>(qMax(0, keysym)),
                 static_cast<uint>(modifiers)});
}

QVariantMap ShortcutsModel::removeAccelerator(const QString &action, int keysym, int modifiers)
{
    return call(QStringLiteral("removeAccelerator"),
                {action, static_cast<uint>(keysym), static_cast<uint>(modifiers)});
}

QVariantMap ShortcutsModel::restoreDefault(const QString &action)
{
    return call(QStringLiteral("restoreDefault"), {action});
}

QVariantMap ShortcutsModel::restoreAllDefaults()
{
    return call(QStringLiteral("restoreAllDefaults"), {});
}

void ShortcutsModel::setRecording(bool recording)
{
    if (m_interface && m_interface->isValid())
        m_interface->asyncCall(QStringLiteral("preventProcessingAccelerators"), recording);
}

QStringList ShortcutsModel::modifierParts(int modifiers) const
{
    // Under the Command convention Meta is the Command key next to the space
    // bar, Alt is Option and Ctrl is Control.
    QStringList parts;
    if (modifiers & ControlDown)
        parts << (m_commandKeys ? tr("Control") : tr("Ctrl"));
    if (modifiers & AltDown)
        parts << (m_commandKeys ? tr("Option") : tr("Alt"));
    if (modifiers & ShiftDown)
        parts << tr("Shift");
    if (modifiers & MetaDown)
        parts << (m_commandKeys ? tr("Command") : tr("Meta"));
    return parts;
}

QStringList ShortcutsModel::partsForKey(int keycode, int qtKey, int modifiers)
{
    QStringList parts = modifierParts(modifiers);
    if (isModifierKey(qtKey))
        return parts;
    QString display;
    if (m_interface && m_interface->isValid()) {
        const QDBusReply<QString> reply =
                m_interface->call(QStringLiteral("describeKey"), static_cast<uint>(qMax(0, keycode)),
                                  static_cast<uint>(keysymForQtKey(qtKey)));
        if (reply.isValid()) {
            const QJsonObject object = QJsonDocument::fromJson(reply.value().toUtf8()).object();
            display = keyDisplay(object.value(QStringLiteral("keyName")).toString(),
                                 object.value(QStringLiteral("keyText")).toString());
        }
    }
    if (display.isEmpty())
        display = QKeySequence(qtKey).toString(QKeySequence::NativeText);
    parts << display;
    return parts;
}

bool ShortcutsModel::isModifierKey(int qtKey) const
{
    switch (qtKey) {
    case Qt::Key_Shift:
    case Qt::Key_Control:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
    case Qt::Key_Meta:
    case Qt::Key_Super_L:
    case Qt::Key_Super_R:
    case Qt::Key_Hyper_L:
    case Qt::Key_Hyper_R:
    case Qt::Key_CapsLock:
    case Qt::Key_NumLock:
    case Qt::Key_ScrollLock:
    case Qt::Key_unknown:
        return true;
    default:
        return false;
    }
}

int ShortcutsModel::compositorModifiers(int qtModifiers) const
{
    int modifiers = 0;
    if (qtModifiers & Qt::ShiftModifier)
        modifiers |= ShiftDown;
    if (qtModifiers & Qt::ControlModifier)
        modifiers |= ControlDown;
    if (qtModifiers & Qt::AltModifier)
        modifiers |= AltDown;
    if (qtModifiers & Qt::MetaModifier)
        modifiers |= MetaDown;
    return modifiers;
}

int ShortcutsModel::keysymForQtKey(int qtKey) const
{
    for (const QtKeysym &entry : kQtKeysyms) {
        if (entry.qtKey == qtKey)
            return entry.keysym;
    }
    if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F35)
        return 0xffbe + (qtKey - Qt::Key_F1);
    // Printable Latin-1 keys are their character, upper-cased by Qt; the
    // compositor lower-cases them.
    if (qtKey >= 0x20 && qtKey <= 0xff)
        return qtKey;
    return 0;
}

QVariantMap ShortcutsModel::action(const QString &name) const
{
    for (const Action &candidate : m_actions) {
        if (candidate.name != name)
            continue;
        return {{QStringLiteral("description"), candidate.description},
                {QStringLiteral("accelerators"), candidate.accelerators},
                {QStringLiteral("isDefault"), candidate.accelerators == candidate.defaults}};
    }
    return {};
}

QString ShortcutsModel::descriptionFor(const QString &action) const
{
    for (const Action &candidate : m_actions) {
        if (candidate.name == action)
            return candidate.description;
    }
    return action;
}
