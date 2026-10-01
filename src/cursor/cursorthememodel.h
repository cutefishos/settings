// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CURSORTHEMEMODEL_H
#define CURSORTHEMEMODEL_H

#include <QAbstractListModel>
#include <QSettings>

class QDir;
class CursorTheme;

class CursorThemeModel : public QAbstractListModel
{
    Q_OBJECT

public:
    Q_PROPERTY(
            QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)
    Q_PROPERTY(int cursorSize READ cursorSize WRITE setCursorSize NOTIFY cursorSizeChanged)

    enum DataRoles {
        NameRole = Qt::UserRole + 1,
        ImageRole,
        PathRole,
        IdRole
    };

    explicit CursorThemeModel(QObject *parent = nullptr);
    ~CursorThemeModel();

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &) const override;
    QVariant data(const QModelIndex &index, int role) const override;

    QString currentTheme() const;
    void setCurrentTheme(const QString &theme);
    int cursorSize() const;
    void setCursorSize(int size);

    Q_INVOKABLE int themeIndex(const QString &theme);

signals:
    void currentThemeChanged();
    void cursorSizeChanged();

private slots:
    void reload();

private:
    void initThemes();
    QStringList paths();
    void processDir(const QDir &dir);

private:
    void apply(const QString &method, const QVariant &value);
    QList<CursorTheme *> m_list;
    QString m_defaultTheme;
    QString m_currentTheme;
    QSettings m_settings;
};

#endif // CURSORTHEMEMODEL_H
