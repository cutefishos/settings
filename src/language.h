#ifndef LOCALE_H
#define LOCALE_H

#include <QObject>
#include <QDBusInterface>

class Language : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList languages READ languages NOTIFY loadLanguageFinished)
    Q_PROPERTY(int currentLanguage READ currentLanguage NOTIFY currentLanguageChanged)

public:
    explicit Language(QObject *parent = nullptr);

    int currentLanguage() const;
    Q_INVOKABLE void setCurrentLanguage(int index);
    QStringList languages() const;

signals:
    void loadLanguageFinished();
    void currentLanguageChanged();

private:
    QDBusInterface m_interface;
    QStringList m_languageNames;
    QStringList m_languageCodes;
    int m_currentLanguage;
};

#endif // LOCALE_H
