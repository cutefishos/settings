// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cstdio>
#include <memory>
#include <string>

#include "ui/geometry.h"
#include "ui/view.h"
#include "ui/window.h"
#include "window/settingswindow.h"

namespace settings::testing {

// The layout dump's line of the view with this name.
inline std::string dumpLine(const cutefish::ui::Window& window, const std::string& name)
{
    const std::string dump = window.layoutDump();
    const size_t at = dump.find("\"" + name + "\"");
    if (at == std::string::npos)
        return {};
    const size_t start = dump.rfind('\n', at) + 1;
    return dump.substr(start, dump.find('\n', at) - start);
}

// The view's logical rect in window coordinates, from its device pixels.
inline cutefish::ui::RectF windowRect(const cutefish::ui::Window& window, const std::string& name)
{
    const std::string line = dumpLine(window, name);
    const size_t device = line.find(" device ");
    if (device == std::string::npos)
        return {-1, -1, -1, -1};
    long x, y, width, height;
    sscanf(line.c_str() + device, " device %ld %ld %ld %ld", &x, &y, &width, &height);
    const double scale = window.scale();
    return {x / scale, y / scale, width / scale, height / scale};
}

inline bool isHidden(const cutefish::ui::Window& window, const std::string& name)
{
    return dumpLine(window, name).ends_with(" hidden");
}

template<typename T = cutefish::ui::View>
T* findView(cutefish::ui::View* view, const std::string& name)
{
    if (view->name() == name)
        return dynamic_cast<T*>(view);
    for (const auto& child : view->children()) {
        if (T* found = findView<T>(child.get(), name))
            return found;
    }
    return nullptr;
}

} // namespace settings::testing

// Opens a page in a headless settings window at 150%, as the references show
// it, and reads its layout back.
class PageFixture
{
public:
    explicit PageFixture(const std::string& name, const std::string& subPage = {})
        : window(std::make_unique<settings::SettingsWindow>())
    {
        window->window().setScale(1.5);
        window->window().resize({900, 820});
        window->switchToPage(name);
        if (!subPage.empty())
            window->pushPage(subPage);
        window->reveal();
        window->window().grab();
    }

    std::string dump() const { return window->window().layoutDump(); }

    std::unique_ptr<settings::SettingsWindow> window;
};
