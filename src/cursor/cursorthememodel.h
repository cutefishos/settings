/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
    Q_PROPERTY(QString currentTheme READ currentTheme WRITE setCurrentTheme NOTIFY currentThemeChanged)

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

    Q_INVOKABLE int themeIndex(const QString &theme);

signals:
    void currentThemeChanged();

private:
    void initThemes();
    QStringList paths();
    void processDir(const QDir &dir);

private:
    QList<CursorTheme *> m_list;
    QString m_defaultTheme;
    QString m_currentTheme;
    QSettings m_settings;
};

#endif // CURSORTHEMEMODEL_H
