#include "iconthememodel.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QDBusServiceWatcher>
#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QPainter>
#include <QStandardPaths>

#include <preferences.h>

// Rendered at 2x so the preview stays crisp on hiDPI; IconItem scales it to fit.
static constexpr int PreviewCell = 22;
static constexpr int PreviewGap = 6;
static constexpr int PreviewScale = 2;
static const QStringList PreviewIcons = {QStringLiteral("folder"), QStringLiteral("text-x-generic"),
                                         QStringLiteral("image-x-generic"), QStringLiteral("user-trash")};

static QStringList iconBasePaths()
{
    QStringList paths;
    paths << QDir::homePath() + QStringLiteral("/.icons");
    for (const QString &path :
         QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation))
        paths << path + QStringLiteral("/icons");
    paths.removeDuplicates();
    return paths;
}

static QString indexFile(const QStringList &basePaths, const QString &themeId)
{
    for (const QString &base : basePaths) {
        const QString path = base + QLatin1Char('/') + themeId + QStringLiteral("/index.theme");
        if (QFileInfo::exists(path))
            return path;
    }
    return QString();
}

IconThemeModel::IconThemeModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_basePaths(iconBasePaths())
    , m_settings(Preferences::file(QStringLiteral("services")), QSettings::IniFormat)
{
    m_settings.beginGroup(QStringLiteral("Appearance"));
    initThemes();

    m_currentTheme = m_settings.value("IconTheme", "Cutefish").toString();
    auto bus = QDBusConnection::sessionBus();
    bus.connect("com.cutefish.Services", "/com/cutefish/Services/Appearance",
                "com.cutefish.Services.Appearance", "iconThemeChanged", this, SLOT(reload()));
    auto *watcher = new QDBusServiceWatcher("com.cutefish.Services", bus,
                                            QDBusServiceWatcher::WatchForOwnerChange, this);
    connect(watcher, &QDBusServiceWatcher::serviceOwnerChanged, this, &IconThemeModel::reload);
}

QHash<int, QByteArray> IconThemeModel::roleNames() const
{
    QHash<int, QByteArray> roleNames = QAbstractListModel::roleNames();
    roleNames[NameRole] = "name";
    roleNames[ImageRole] = "image";
    roleNames[IdRole] = "id";
    return roleNames;
}

int IconThemeModel::rowCount(const QModelIndex &) const
{
    return m_themes.size();
}

QVariant IconThemeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_themes.size())
        return QVariant();

    const Theme &theme = m_themes.at(index.row());

    switch (role) {
    case NameRole:
        return theme.name;
    case ImageRole:
        return theme.preview;
    case IdRole:
        return theme.id;
    default:
        return QVariant();
    }
}

QString IconThemeModel::currentTheme() const
{
    return m_currentTheme;
}

void IconThemeModel::setCurrentTheme(const QString &theme)
{
    if (m_currentTheme == theme || themeIndex(theme) < 0)
        return;

    m_currentTheme = theme;
    emit currentThemeChanged();

    auto message = QDBusMessage::createMethodCall("com.cutefish.Services",
                                                  "/com/cutefish/Services/Appearance",
                                                  "com.cutefish.Services.Appearance",
                                                  "setIconTheme");
    message << theme;
    auto *watcher =
            new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
            [this](QDBusPendingCallWatcher *call) {
                const QDBusPendingReply<> reply = *call;
                if (reply.isError())
                    qWarning() << "Cannot apply icon theme:" << reply.error().message();
                reload();
                call->deleteLater();
            });
}

int IconThemeModel::themeIndex(const QString &theme) const
{
    for (int i = 0; i < m_themes.size(); ++i) {
        if (m_themes.at(i).id == theme)
            return i;
    }
    return -1;
}

void IconThemeModel::reload()
{
    m_settings.sync();
    m_currentTheme = m_settings.value("IconTheme", "Cutefish").toString();
    emit currentThemeChanged();
}

void IconThemeModel::initThemes()
{
    QStringList seen;
    for (const QString &base : m_basePaths) {
        const QDir dir(base);
        for (const QString &id : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if (seen.contains(id) || !QFileInfo::exists(dir.filePath(id + "/index.theme")))
                continue;
            seen << id;

            QSettings index(dir.filePath(id + "/index.theme"), QSettings::IniFormat);
            index.beginGroup("Icon Theme");
            // Cursor themes and fallback themes such as hicolor carry index.theme too.
            if (index.value("Hidden", false).toBool() || !index.contains("Directories"))
                continue;

            Theme theme;
            theme.id = id;
            theme.name = index.value("Name", id).toString();
            theme.inherits = index.value("Inherits").toStringList();
            theme.preview = renderPreview(theme);
            m_themes << theme;
        }
    }

    std::sort(m_themes.begin(), m_themes.end(), [](const Theme &a, const Theme &b) {
        return a.name.localeAwareCompare(b.name) < 0;
    });
}

QString IconThemeModel::findIcon(const QString &themeId, const QString &name,
                                 QStringList &visited) const
{
    if (visited.contains(themeId))
        return QString();
    visited << themeId;

    const QString indexPath = indexFile(m_basePaths, themeId);
    if (indexPath.isEmpty())
        return QString();

    QSettings index(indexPath, QSettings::IniFormat);
    const QStringList directories =
            index.value("Icon Theme/Directories").toStringList();

    // Scalable artwork first, then the fixed size closest to the render size.
    QList<QPair<int, QString>> ranked;
    for (const QString &directory : directories) {
        const QString type = index.value(directory + "/Type", "Threshold").toString();
        const int size = index.value(directory + "/Size", 0).toInt();
        const int rank = type == QLatin1String("Scalable")
                ? 0
                : 1 + qAbs(size - PreviewCell * PreviewScale);
        ranked << qMakePair(rank, directory);
    }
    std::sort(ranked.begin(), ranked.end());

    for (const auto &entry : ranked) {
        for (const QString &base : m_basePaths) {
            const QString prefix = base + QLatin1Char('/') + themeId + QLatin1Char('/')
                    + entry.second + QLatin1Char('/') + name;
            for (const char *suffix : {".svg", ".png"}) {
                if (QFileInfo::exists(prefix + QLatin1String(suffix)))
                    return prefix + QLatin1String(suffix);
            }
        }
    }

    QStringList parents = index.value("Icon Theme/Inherits").toStringList();
    parents << QStringLiteral("hicolor");
    for (const QString &parent : parents) {
        const QString found = findIcon(parent, name, visited);
        if (!found.isEmpty())
            return found;
    }
    return QString();
}

QPixmap IconThemeModel::renderPreview(const Theme &theme) const
{
    const int cell = PreviewCell * PreviewScale;
    const int gap = PreviewGap * PreviewScale;
    const int extent = cell * 2 + gap;

    QPixmap preview(extent, extent);
    preview.fill(Qt::transparent);

    QPainter painter(&preview);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    for (int i = 0; i < PreviewIcons.size(); ++i) {
        QStringList visited;
        const QString file = findIcon(theme.id, PreviewIcons.at(i), visited);
        if (file.isEmpty())
            continue;

        QImageReader reader(file);
        reader.setScaledSize(QSize(cell, cell));
        const QImage image = reader.read();
        if (image.isNull())
            continue;

        const int x = (i % 2) * (cell + gap);
        const int y = (i / 2) * (cell + gap);
        painter.drawImage(QRect(x, y, cell, cell), image);
    }
    painter.end();

    // The painter works in device pixels; the ratio must be set afterwards.
    preview.setDevicePixelRatio(PreviewScale);
    return preview;
}
