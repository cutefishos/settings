#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDBusConnection>

class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);

    int run();

    void switchToPage(const QString &name);
    void joinWifi(const QString &accessPoint);

private:
    QQmlApplicationEngine m_engine;
};

#endif // APPLICATION_H
