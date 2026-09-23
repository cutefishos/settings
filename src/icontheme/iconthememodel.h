#ifndef ICONTHEMEMODEL_H
#define ICONTHEMEMODEL_H

#include <QAbstractListModel>
#include <QPixmap>
#include <QSettings>

class IconThemeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(
            QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)

public:
    enum DataRoles {
        NameRole = Qt::UserRole + 1,
        ImageRole,
        IdRole
    };

    explicit IconThemeModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QString currentTheme() const;
    void setCurrentTheme(const QString &theme);

    Q_INVOKABLE int themeIndex(const QString &theme) const;

signals:
    void currentThemeChanged();

private slots:
    void reload();

private:
    struct Theme
    {
        QString id;
        QString name;
        QStringList inherits;
        QPixmap preview;
    };

    void initThemes();
    QString findIcon(const QString &themeId, const QString &name, QStringList &visited) const;
    QPixmap renderPreview(const Theme &theme) const;

    QStringList m_basePaths;
    QList<Theme> m_themes;
    QString m_currentTheme;
    QSettings m_settings;
};

#endif // ICONTHEMEMODEL_H
