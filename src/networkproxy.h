#ifndef NETWORKPROXY_H
#define NETWORKPROXY_H

#include "proxysettings.h"

#include <QFileSystemWatcher>
#include <QObject>
#include <QTimer>

// Edits org.gnome.system.proxy, the store third-party proxy clients write too, and follows
// their changes while the page is open.
class NetworkProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int flag READ flag WRITE setFlag NOTIFY flagChanged)
    Q_PROPERTY(bool useSameProxy READ useSameProxy WRITE setUseSameProxy NOTIFY useSameProxyChanged)
    Q_PROPERTY(QString scriptProxy READ scriptProxy WRITE setScriptProxy NOTIFY scriptProxyChanged)
    Q_PROPERTY(QString httpProxy READ httpProxy WRITE setHttpProxy NOTIFY httpProxyChanged)
    Q_PROPERTY(QString ftpProxy READ ftpProxy WRITE setFtpProxy NOTIFY ftpProxyChanged)
    Q_PROPERTY(QString socksProxy READ socksProxy WRITE setSocksProxy NOTIFY socksProxyChanged)
    Q_PROPERTY(QString httpProxyPort READ httpProxyPort WRITE setHttpProxyPort NOTIFY httpProxyPortChanged)
    Q_PROPERTY(QString ftpProxyPort READ ftpProxyPort WRITE setFtpProxyPort NOTIFY ftpProxyPortChanged)
    Q_PROPERTY(QString socksProxyPort READ socksProxyPort WRITE setSocksProxyPort NOTIFY socksProxyPortChanged)

public:
    explicit NetworkProxy(QObject *parent = nullptr);

    int flag() const;
    void setFlag(int flag);

    bool useSameProxy() const;
    void setUseSameProxy(bool enabled);

    QString scriptProxy() const;
    void setScriptProxy(const QString &path);

    QString httpProxy() const;
    void setHttpProxy(const QString &path);

    QString ftpProxy() const;
    void setFtpProxy(const QString &path);

    QString socksProxy() const;
    void setSocksProxy(const QString &path);

    QString httpProxyPort() const;
    void setHttpProxyPort(const QString &port);

    QString ftpProxyPort() const;
    void setFtpProxyPort(const QString &port);

    QString socksProxyPort() const;
    void setSocksProxyPort(const QString &port);

signals:
    void flagChanged();
    void useSameProxyChanged();
    void scriptProxyChanged();
    void httpProxyChanged();
    void ftpProxyChanged();
    void socksProxyChanged();
    void httpProxyPortChanged();
    void ftpProxyPortChanged();
    void socksProxyPortChanged();

private:
    void watch();
    void reload();

    QFileSystemWatcher m_watcher;
    QTimer m_reloadTimer;
    // The page sets every field in a row on Save; one write covers them all.
    QTimer m_saveTimer;
    ProxySettings m_settings;
    bool m_useSameProxy = false;
};

#endif // NETWORKPROXY_H
