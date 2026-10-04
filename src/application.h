#ifndef APPLICATION_H
#define APPLICATION_H

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDBusConnection>

class Application : public QGuiApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);

    void switchToPage(const QString &name);
    void joinWifi(const QString &accessPoint);

private:
    QQmlApplicationEngine m_engine;
};

#endif // APPLICATION_H
