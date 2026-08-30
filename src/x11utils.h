#pragma once

#include <QGuiApplication>
#include <QScreen>
namespace Cutefish::X11
{
inline int dpiY()
{
    const auto screen = qGuiApp ? qGuiApp->primaryScreen() : nullptr;
    return screen ? qRound(screen->logicalDotsPerInchY()) : 96;
}
}
