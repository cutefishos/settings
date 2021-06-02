#include "language.h"
#include <QLocale>
#include <QDebug>
#include <QDir>

#include <unicode/locid.h>
#include <unicode/unistr.h>

static const QStringList supportList = {"en_US", "en_AU", "en_CA", "en_NZ", "en_GB",
                                        "es_MX", "ja", "ko", "pl", "pt_BR", "ru", "de", "it", 
                                        "zh_CN", "zh_HK", "zh_TW"
                                       };

static const QMap<QString, QString> s_displayName = {   {"zh_CN", "简体中文"},
                                                        {"zh_HK", "繁體中文(香港)"},
                                                        {"zh_TW", "正體中文"}
                                                  };

Language::Language(QObject *parent)
    : QObject(parent)
    , m_interface("org.cutefish.Settings",
                  "/Language",
                  "org.cutefish.Language",
                  QDBusConnection::sessionBus())
    , m_currentLanguage(-1)
{
    QString systemLanguage = m_interface.property("languageCode").toString();

    for (const QString &code : supportList) {
        std::string string;
        icu::Locale locale = qPrintable(code);
        icu::UnicodeString unicodeString;
        locale.getDisplayName(locale, unicodeString);
        unicodeString.toUTF8String(string);

        QString displayName = s_displayName.contains(code) ? s_displayName.value(code)
                                                           : string.c_str();

        if (displayName.isEmpty() || displayName == "C")
            continue;

        m_languageNames.append(displayName);
        m_languageCodes.append(code);
    }

    // Update current language
    m_currentLanguage = m_languageCodes.indexOf(systemLanguage);

    emit loadLanguageFinished();
    emit currentLanguageChanged();
}

int Language::currentLanguage() const
{
    return m_currentLanguage;
}

void Language::setCurrentLanguage(int index)
{
    if (index >= 0 && index < m_languageCodes.length()) {
        m_interface.call("setLanguage", m_languageCodes[index]);
        qDebug() << "set language: " << m_languageCodes[index];
        m_currentLanguage = index;
        emit currentLanguageChanged();
    }
}

QStringList Language::languages() const
{
    return m_languageNames;
}
