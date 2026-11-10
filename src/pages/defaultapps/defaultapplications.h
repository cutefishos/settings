// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>

#include "base/file_watcher.h"
#include "base/signal.h"
#include "base/timer.h"

namespace settings {

// The applications that can be the default web browser, mail client, text
// editor, music player, video player, image viewer and terminal, and which
// ones are; an installed bundle counts where it opens the main type. The
// choices go to the user's mimeapps.list and the terminal list.
class DefaultApplications
{
public:
    enum Kind {
        Browser,
        Email,
        TextEditor,
        MusicPlayer,
        VideoPlayer,
        ImageViewer,
        Terminal,
        KindCount,
    };

    struct Application
    {
        std::string path;
        // How mimeapps.list names it: the desktop file's name, or the
        // bundle's path.
        std::string fileName;
        std::string name;
        std::string icon;
        std::vector<std::string> mimeTypes;
    };

    DefaultApplications();

    const std::vector<Application>& applications(Kind kind) const { return m_lists[kind]; }
    // -1 when none of them is the default.
    int defaultIndex(Kind kind) const { return m_defaults[kind]; }
    void setDefault(Kind kind, int index);

    // After the lists or the defaults changed.
    cutefish::base::Signal<> changed;

private:
    void load();
    void watchLists();
    std::vector<std::string> textTypes(const Application& editor) const;

    std::vector<Application> m_lists[KindCount];
    int m_defaults[KindCount] = {-1, -1, -1, -1, -1, -1, -1};
    cutefish::base::Subscription m_registry;
    std::vector<cutefish::base::FileWatch> m_watches;
    cutefish::base::Timer m_reload;
};

} // namespace settings
