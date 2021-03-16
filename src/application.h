#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDBusConnection>

class Application : public QApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);

    void switchToPage(const QString &name);

private:
    QQmlApplicationEngine m_engine;
};

#endif // APPLICATION_H
