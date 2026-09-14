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

#include "language.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusPendingReply>
#include <QFutureWatcher>
#include <QLocale>
#include <QSettings>
#include <QtConcurrent>

#include <algorithm>
#include <unicode/locid.h>
#include <unicode/unistr.h>

static QString displayName(const icu::Locale &locale, const icu::Locale &displayLocale)
{
    icu::UnicodeString name;
    locale.getDisplayName(displayLocale, name);
    std::string utf8;
    name.toUTF8String(utf8);
    return QString::fromUtf8(utf8.data(), utf8.size());
}

static QList<LanguageEntry> languageCatalog(const QString &displayCode)
{
    static const QStringList supportedSystemLanguages = {
        "en_US",       // English (US)
        "en_GB",       // English (UK)
        "ja_JP",       // Japanese
        "ko_KR",       // Korean
        "fr_FR",       // French
        "de_DE",       // German
        "es_ES",       // Spanish (Spain)
        "pt_BR",       // Portuguese (Brazil)
        "ru_RU",       // Russian
        "it_IT",       // Italian
        "id_ID",       // Indonesian
        "vi_VN",       // Vietnamese
        "th_TH",       // Thai
        "ms_MY",       // Malay
        "hi_IN",       // Hindi
        "bn_BD",       // Bangla
        "tr_TR",       // Turkish
        "uk_UA",       // Ukrainian
        "pl_PL",       // Polish
        "nl_NL",       // Dutch
        "fr_CA",       // French (Canada)
        "pt_PT",       // Portuguese (Portugal)
        "es_419",      // Spanish (Latin America)
        "es_MX",       // Spanish (Mexico)
        "cs_CZ",       // Czech
        "da_DK",       // Danish
        "fi_FI",       // Finnish
        "sv_SE",       // Swedish
        "nb_NO",       // Norwegian Bokmål
        "hu_HU",       // Hungarian
        "ro_RO",       // Romanian
        "sk_SK",       // Slovak
        "sl_SI",       // Slovenian
        "hr_HR",       // Croatian
        "bg_BG",       // Bulgarian
        "ca_ES",       // Catalan
        "el_GR",       // Greek
        "ar",          // Arabic
        "he_IL",       // Hebrew
        "zh_Hans_CN",  // Chinese, Simplified
        "zh_Hant_HK",  // Chinese, Traditional (Hong Kong)
        "zh_Hant_TW",  // Chinese, Traditional (Taiwan)
    };
    QList<LanguageEntry> entries;
    const icu::Locale displayLocale(displayCode.toUtf8().constData());
    for (const QString &code : supportedSystemLanguages) {
        const icu::Locale locale(code.toUtf8().constData());
        if (locale.isBogus() || !*locale.getLanguage())
            continue;
        LanguageEntry entry;
        entry.code = code;
        entry.name = displayName(locale, locale);
        entry.description = displayName(locale, displayLocale);
        entry.searchText = entry.name + ' ' + entry.description + ' ' + entry.code;
        entries.append(entry);
    }
    return entries;
}

LanguageModel::LanguageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int LanguageModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_entries.size();
}

QVariant LanguageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_entries.size())
        return {};
    const auto &entry = m_entries.at(index.row());
    switch (role) {
    case CodeRole: return entry.code;
    case NameRole: return entry.name;
    case DescriptionRole: return entry.description;
    case SearchRole: return entry.searchText;
    case AddedRole: return m_preferredCodes.contains(entry.code);
    default: return {};
    }
}

QHash<int, QByteArray> LanguageModel::roleNames() const
{
    return {{CodeRole, "code"}, {NameRole, "name"}, {DescriptionRole, "description"},
            {SearchRole, "searchText"}, {AddedRole, "added"}};
}

void LanguageModel::setEntries(const QList<LanguageEntry> &entries)
{
    beginResetModel();
    m_entries = entries;
    endResetModel();
}

void LanguageModel::setPreferredCodes(const QStringList &codes)
{
    if (m_preferredCodes == codes)
        return;
    m_preferredCodes = codes;
    if (!m_entries.isEmpty())
        emit dataChanged(index(0), index(m_entries.size() - 1), {AddedRole});
}

bool LanguageModel::contains(const QString &code) const
{
    return std::any_of(m_entries.cbegin(), m_entries.cend(), [&code](const auto &entry) {
        return entry.code == code;
    });
}

AvailableLanguageModel::AvailableLanguageModel(LanguageModel *source)
{
    setSourceModel(source);
    setFilterRole(LanguageModel::SearchRole);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
    connect(source, &QAbstractItemModel::dataChanged, this,
            [this](const QModelIndex &, const QModelIndex &, const QList<int> &roles) {
        if (roles.isEmpty() || roles.contains(LanguageModel::AddedRole))
            invalidateFilter();
    });
}

bool AvailableLanguageModel::filterAcceptsRow(int row, const QModelIndex &parent) const
{
    const QModelIndex index = sourceModel()->index(row, 0, parent);
    return !sourceModel()->data(index, LanguageModel::AddedRole).toBool()
           && QSortFilterProxyModel::filterAcceptsRow(row, parent);
}

Language::Language(QObject *parent)
    : QObject(parent)
    , m_filteredLanguages(&m_languages)
{
    reload();
    QDBusConnection::sessionBus().connect("com.cutefish.Services", "/com/cutefish/Services/Language",
                                         "com.cutefish.Services.Language", "languageChanged",
                                         this, SLOT(reload()));
}

void Language::loadLanguages()
{
    if (m_loading || m_loaded)
        return;
    m_loading = true;
    emit loadingChanged();

    // Reopening the page reuses the catalog without repeating ICU work.
    static QHash<QString, QFuture<QList<LanguageEntry>>> catalogs;
    const QString displayCode = QLocale().name();
    if (!catalogs.contains(displayCode))
        catalogs.insert(displayCode, QtConcurrent::run(languageCatalog, displayCode));

    auto *watcher = new QFutureWatcher<QList<LanguageEntry>>(this);
    connect(watcher, &QFutureWatcherBase::finished, this, [this, watcher]() {
        m_languages.setEntries(watcher->result());
        m_loaded = true;
        m_loading = false;
        emit loadingChanged();
        watcher->deleteLater();
    });
    watcher->setFuture(catalogs.value(displayCode));
}

void Language::search(const QString &text)
{
    m_filteredLanguages.setFilterFixedString(text.trimmed());
}

void Language::reload()
{
    QSettings settings("cutefish", "language");
    QString primary = settings.value("language", QLocale::system().name()).toString();
    QStringList codes = settings.value("preferredLanguages").toStringList();
    codes.removeAll(primary);
    codes.prepend(primary);
    codes.removeDuplicates();
    if (codes == m_preferredCodes)
        return;
    m_preferredCodes = codes;
    m_languages.setPreferredCodes(codes);
    emit preferredLanguagesChanged();
}

QVariantList Language::preferredLanguages() const
{
    QVariantList entries;
    const icu::Locale displayLocale(QLocale().name().toUtf8().constData());
    for (const QString &code : m_preferredCodes) {
        const icu::Locale locale(code.toUtf8().constData());
        entries.append(QVariantMap{{"code", code}, {"name", displayName(locale, locale)},
                                   {"description", displayName(locale, displayLocale)}});
    }
    return entries;
}

void Language::addLanguage(const QString &code)
{
    if (!m_languages.contains(code) || m_preferredCodes.contains(code))
        return;
    QStringList codes = m_preferredCodes;
    codes.append(code);
    saveLanguages(codes);
}

void Language::removeLanguage(int index)
{
    if (m_preferredCodes.size() <= 1 || index < 0 || index >= m_preferredCodes.size())
        return;
    QStringList codes = m_preferredCodes;
    codes.removeAt(index);
    saveLanguages(codes);
}

void Language::moveLanguage(int from, int to)
{
    if (from < 0 || to < 0 || from >= m_preferredCodes.size()
        || to >= m_preferredCodes.size() || from == to)
        return;
    QStringList codes = m_preferredCodes;
    codes.move(from, to);
    saveLanguages(codes);
}

void Language::saveLanguages(const QStringList &codes)
{
    if (m_saving)
        return;
    m_errorMessage.clear();
    emit errorMessageChanged();
    m_saving = true;
    emit savingChanged();

    QDBusMessage message = QDBusMessage::createMethodCall("com.cutefish.Services",
        "/com/cutefish/Services/Language", "com.cutefish.Services.Language", "setPreferredLanguages");
    message << codes;
    auto *watcher = new QDBusPendingCallWatcher(QDBusConnection::sessionBus().asyncCall(message), this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, codes]() {
        QDBusPendingReply<> reply = *watcher;
        if (reply.isError()) {
            m_errorMessage = tr("Could not save preferred languages. Please try again.");
            emit errorMessageChanged();
        } else {
            m_preferredCodes = codes;
            m_languages.setPreferredCodes(codes);
            emit preferredLanguagesChanged();
        }
        m_saving = false;
        emit savingChanged();
        watcher->deleteLater();
    });
}
