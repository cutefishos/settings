#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <QObject>
#include <QList>
#include <QVariant>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDirIterator>
#include <QDir>

class Background : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString currentBackgroundPath READ currentBackgroundPath WRITE setBackground NOTIFY backgroundChanged)
    Q_PROPERTY(QVariantList backgrounds READ backgrounds NOTIFY stub)

public:
    explicit Background(QObject *parent = nullptr);

    QVariantList backgrounds();
    QString currentBackgroundPath();
    Q_INVOKABLE void setBackground(QString newBackgroundPath);

signals:
    void backgroundChanged();
    void stub();

private:
    QString m_currentPath;
};

#endif
