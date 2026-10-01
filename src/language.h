// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOCALE_H
#define LOCALE_H

#include <QObject>
#include <QAbstractListModel>
#include <QSortFilterProxyModel>
#include <QVariantList>

struct LanguageEntry
{
    QString code;
    QString name;
    QString description;
    QString searchText;
};

class LanguageModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Role { CodeRole = Qt::UserRole + 1, NameRole, DescriptionRole, SearchRole, AddedRole };
    explicit LanguageModel(QObject *parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;
    void setEntries(const QList<LanguageEntry> &entries);
    void setPreferredCodes(const QStringList &codes);
    bool contains(const QString &code) const;

private:
    QList<LanguageEntry> m_entries;
    QStringList m_preferredCodes;
};

class AvailableLanguageModel : public QSortFilterProxyModel
{
public:
    explicit AvailableLanguageModel(LanguageModel *source);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
};

class Language : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList preferredLanguages READ preferredLanguages NOTIFY preferredLanguagesChanged)
    Q_PROPERTY(QAbstractItemModel *availableLanguages READ availableLanguages CONSTANT)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(bool saving READ saving NOTIFY savingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)

public:
    explicit Language(QObject *parent = nullptr);

    QVariantList preferredLanguages() const;
    QAbstractItemModel *availableLanguages() { return &m_filteredLanguages; }
    bool loading() const { return m_loading; }
    bool saving() const { return m_saving; }
    QString errorMessage() const { return m_errorMessage; }
    Q_INVOKABLE void loadLanguages();
    Q_INVOKABLE void search(const QString &text);
    Q_INVOKABLE void addLanguage(const QString &code);
    Q_INVOKABLE void removeLanguage(int index);
    Q_INVOKABLE void moveLanguage(int from, int to);

signals:
    void preferredLanguagesChanged();
    void loadingChanged();
    void savingChanged();
    void errorMessageChanged();

private:
    void saveLanguages(const QStringList &codes);
    Q_SLOT void reload();

    LanguageModel m_languages;
    AvailableLanguageModel m_filteredLanguages;
    QStringList m_preferredCodes;
    bool m_loaded = false;
    bool m_loading = false;
    bool m_saving = false;
    QString m_errorMessage;
};

#endif // LOCALE_H
