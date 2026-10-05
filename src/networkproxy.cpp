#include "networkproxy.h"
#include "keyfile.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QUrl>

static QString portText(const ProxySettings::Server &server)
{
    return server.host.isEmpty() || server.port <= 0 ? QString() : QString::number(server.port);
}

static int portValue(const QString &text)
{
    bool ok = false;
    const int port = text.toInt(&ok);
    return ok && port > 0 && port < 65536 ? port : 0;
}

static bool sameServer(const ProxySettings &settings)
{
    return !settings.http.host.isEmpty() && settings.ftp == settings.http;
}

NetworkProxy::NetworkProxy(QObject *parent)
    : QObject(parent)
    , m_settings(ProxySettings::load())
    , m_useSameProxy(sameServer(m_settings))
{
    m_saveTimer.setSingleShot(true);
    m_saveTimer.setInterval(0);
    connect(&m_saveTimer, &QTimer::timeout, this, [this] { m_settings.save(); });

    // gsettings replaces the file by rename, which drops a file watch.
    m_reloadTimer.setSingleShot(true);
    m_reloadTimer.setInterval(100);
    connect(&m_reloadTimer, &QTimer::timeout, this, &NetworkProxy::reload);
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &NetworkProxy::watch);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &NetworkProxy::watch);

    const QString path = GSettingsKeyfile::path();
    QDir().mkpath(QFileInfo(path).absolutePath());
    m_watcher.addPath(QFileInfo(path).absolutePath());
    if (QFile::exists(path))
        m_watcher.addPath(path);
}

int NetworkProxy::flag() const
{
    switch (m_settings.mode) {
    case ProxySettings::Auto:
        return 1;
    case ProxySettings::Manual:
        return 2;
    default:
        return 0;
    }
}

void NetworkProxy::setFlag(int flag)
{
    if (NetworkProxy::flag() != flag) {
        m_settings.mode = flag == 1 ? ProxySettings::Auto
                        : flag == 2 ? ProxySettings::Manual
                                    : ProxySettings::None;
        m_saveTimer.start();
        emit flagChanged();
    }
}

bool NetworkProxy::useSameProxy() const
{
    return m_useSameProxy;
}

void NetworkProxy::setUseSameProxy(bool enabled)
{
    // Only a page state: the FTP fields it fills in are what gets saved.
    if (m_useSameProxy != enabled) {
        m_useSameProxy = enabled;
        emit useSameProxyChanged();
    }
}

QString NetworkProxy::scriptProxy() const
{
    const QUrl url(m_settings.autoconfigUrl);
    return url.isLocalFile() ? url.toLocalFile() : m_settings.autoconfigUrl;
}

void NetworkProxy::setScriptProxy(const QString &path)
{
    // The schema holds a URL; the file dialog hands over a local path.
    const QString url = QDir::isAbsolutePath(path) ? QUrl::fromLocalFile(path).toString() : path;
    if (m_settings.autoconfigUrl != url) {
        m_settings.autoconfigUrl = url;
        m_saveTimer.start();
        emit scriptProxyChanged();
    }
}

QString NetworkProxy::httpProxy() const
{
    return m_settings.http.host;
}

void NetworkProxy::setHttpProxy(const QString &path)
{
    if (m_settings.http.host != path) {
        // The page has no HTTPS field; HTTPS goes through the HTTP proxy.
        m_settings.http.host = path;
        m_settings.https.host = path;
        m_saveTimer.start();
        emit httpProxyChanged();
        emit httpProxyPortChanged();
    }
}

QString NetworkProxy::ftpProxy() const
{
    return m_settings.ftp.host;
}

void NetworkProxy::setFtpProxy(const QString &path)
{
    if (m_settings.ftp.host != path) {
        m_settings.ftp.host = path;
        m_saveTimer.start();
        emit ftpProxyChanged();
        emit ftpProxyPortChanged();
    }
}

QString NetworkProxy::socksProxy() const
{
    return m_settings.socks.host;
}

void NetworkProxy::setSocksProxy(const QString &path)
{
    if (m_settings.socks.host != path) {
        m_settings.socks.host = path;
        m_saveTimer.start();
        emit socksProxyChanged();
        emit socksProxyPortChanged();
    }
}

QString NetworkProxy::httpProxyPort() const
{
    return portText(m_settings.http);
}

void NetworkProxy::setHttpProxyPort(const QString &port)
{
    const int value = portValue(port);
    if (m_settings.http.port != value) {
        m_settings.http.port = value;
        m_settings.https.port = value;
        m_saveTimer.start();
        emit httpProxyPortChanged();
    }
}

QString NetworkProxy::ftpProxyPort() const
{
    return portText(m_settings.ftp);
}

void NetworkProxy::setFtpProxyPort(const QString &port)
{
    const int value = portValue(port);
    if (m_settings.ftp.port != value) {
        m_settings.ftp.port = value;
        m_saveTimer.start();
        emit ftpProxyPortChanged();
    }
}

QString NetworkProxy::socksProxyPort() const
{
    return portText(m_settings.socks);
}

void NetworkProxy::setSocksProxyPort(const QString &port)
{
    const int value = portValue(port);
    if (m_settings.socks.port != value) {
        m_settings.socks.port = value;
        m_saveTimer.start();
        emit socksProxyPortChanged();
    }
}

void NetworkProxy::watch()
{
    const QString path = GSettingsKeyfile::path();
    if (QFile::exists(path) && !m_watcher.files().contains(path))
        m_watcher.addPath(path);
    m_reloadTimer.start();
}

void NetworkProxy::reload()
{
    // An edit still waiting to be written would be lost; its own write triggers another reload.
    if (m_saveTimer.isActive())
        return;

    const ProxySettings previous = m_settings;
    m_settings = ProxySettings::load();
    if (m_settings == previous)
        return;

    if (m_settings.mode != previous.mode)
        emit flagChanged();
    if (m_settings.autoconfigUrl != previous.autoconfigUrl)
        emit scriptProxyChanged();
    if (m_settings.http.host != previous.http.host)
        emit httpProxyChanged();
    if (m_settings.http != previous.http)
        emit httpProxyPortChanged();
    if (m_settings.ftp.host != previous.ftp.host)
        emit ftpProxyChanged();
    if (m_settings.ftp != previous.ftp)
        emit ftpProxyPortChanged();
    if (m_settings.socks.host != previous.socks.host)
        emit socksProxyChanged();
    if (m_settings.socks != previous.socks)
        emit socksProxyPortChanged();
    setUseSameProxy(sameServer(m_settings));
}
