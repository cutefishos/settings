// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/defaultapps/defaultapplications.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>

#include "base/application_registry.h"
#include "base/mime_apps.h"

namespace fs = std::filesystem;
using namespace cutefish::base;

namespace settings {

static bool contains(const std::vector<std::string>& list, const std::string& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

static bool opensAny(const DesktopEntry& entry, const std::string& category, const std::string& prefix)
{
    if (!contains(entry.categories, category))
        return false;
    return std::any_of(entry.mimeTypes.begin(), entry.mimeTypes.end(),
                       [&prefix](const std::string& type) { return type.starts_with(prefix); });
}

DefaultApplications::DefaultApplications()
{
    ApplicationRegistry& registry = ApplicationRegistry::instance();
    m_registry = registry.changed.connect([this] { load(); });
    if (!registry.isLoading())
        load();
    // Other programs, the file manager among them, choose defaults too.
    m_reload.setSingleShot(true);
    m_reload.onTimeout = [this] { load(); };
    watchLists();
}

// Lists are saved by replacing the file; the folder sees them come back.
void DefaultApplications::watchLists()
{
    m_watches.clear();
    const auto reload = [this](auto&&...) { m_reload.start(std::chrono::milliseconds(100)); };
    for (const std::string& path : {mimeapps::userListPath(), mimeapps::userTerminalsListPath()})
        m_watches.push_back(FileWatch::file(path, reload));
    m_watches.push_back(FileWatch::directory(fs::path(mimeapps::userListPath()).parent_path().string(), reload));
}

void DefaultApplications::load()
{
    for (auto& list : m_lists)
        list.clear();
    std::fill(std::begin(m_defaults), std::end(m_defaults), -1);

    for (const auto& entry : ApplicationRegistry::instance().applications()) {
        Application application{entry->path, fs::path(entry->path).filename().string(), entry->name, entry->icon,
                                entry->mimeTypes};
        // A player can be both the music and the video player.
        if (contains(entry->categories, "WebBrowser") && contains(entry->mimeTypes, "x-scheme-handler/http"))
            m_lists[Browser].push_back(application);
        if (contains(entry->categories, "Email") && contains(entry->mimeTypes, "x-scheme-handler/mailto"))
            m_lists[Email].push_back(application);
        if (contains(entry->categories, "TextEditor") && contains(entry->mimeTypes, "text/plain"))
            m_lists[TextEditor].push_back(application);
        if (opensAny(*entry, "AudioVideo", "audio/"))
            m_lists[MusicPlayer].push_back(application);
        if (opensAny(*entry, "AudioVideo", "video/"))
            m_lists[VideoPlayer].push_back(application);
        // Browsers and image editors open images too.
        if (opensAny(*entry, "Viewer", "image/"))
            m_lists[ImageViewer].push_back(application);
        if (contains(entry->categories, "TerminalEmulator"))
            m_lists[Terminal].push_back(application);
    }

    const auto first = [](const std::vector<std::string>& list) { return list.empty() ? std::string() : list.front(); };
    const std::string browser = first(mimeapps::associations(mimeapps::userListPath(), "Default Applications",
                                                             "x-scheme-handler/http"));
    const std::string email = first(mimeapps::associations(mimeapps::userListPath(), "Default Applications",
                                                           "x-scheme-handler/mailto"));
    const std::string terminal = mimeapps::defaultTerminal(mimeapps::userTerminalsListPath());
    // Applications differ in the types they list; the commonest one stands
    // for all. Without a choice, this is what the file manager opens them with.
    const std::string textEditor = mimeapps::defaultApplication(mimeapps::listPaths(), "text/plain");
    const std::string musicPlayer = mimeapps::defaultApplication(mimeapps::listPaths(), "audio/mpeg");
    const std::string videoPlayer = mimeapps::defaultApplication(mimeapps::listPaths(), "video/mp4");
    const std::string imageViewer = mimeapps::defaultApplication(mimeapps::listPaths(), "image/jpeg");

    const auto find = [this](Kind kind, const std::string& value, bool byPath) {
        const auto& list = m_lists[kind];
        for (size_t i = 0; i < list.size(); ++i) {
            if ((byPath ? list[i].path : list[i].fileName) == value) {
                m_defaults[kind] = int(i);
                return;
            }
        }
    };
    find(Browser, browser, false);
    find(Email, email, false);
    find(TextEditor, textEditor, true);
    find(MusicPlayer, musicPlayer, true);
    find(VideoPlayer, videoPlayer, true);
    find(ImageViewer, imageViewer, true);
    find(Terminal, terminal, false);
    changed.emit();
}

void DefaultApplications::setDefault(Kind kind, int index)
{
    const auto& list = m_lists[kind];
    if (index < 0 || index >= int(list.size()))
        return;
    const Application& application = list[size_t(index)];
    const std::string userList = mimeapps::userListPath();
    const auto set = [&userList](const std::string& type, const std::string& desktop) {
        if (!mimeapps::setDefault(userList, type, desktop))
            fprintf(stderr, "Failed to save default application for %s\n", type.c_str());
    };
    const auto setAll = [&set, &application](const std::string& prefix) {
        for (const std::string& type : application.mimeTypes) {
            if (type.starts_with(prefix))
                set(type, application.fileName);
        }
    };
    switch (kind) {
    case Browser:
        set("x-scheme-handler/http", application.fileName);
        set("x-scheme-handler/https", application.fileName);
        break;
    case Email:
        set("x-scheme-handler/mailto", application.fileName);
        break;
    // Only plain text: editors also list web pages and other types that have
    // an application of their own.
    case TextEditor:
        set("text/plain", application.fileName);
        break;
    case MusicPlayer:
        setAll("audio/");
        break;
    case VideoPlayer:
        setAll("video/");
        break;
    case ImageViewer:
        setAll("image/");
        break;
    case Terminal:
        if (!mimeapps::setDefaultTerminal(mimeapps::userTerminalsListPath(), application.fileName)) {
            fprintf(stderr, "Failed to save the default terminal\n");
            return;
        }
        break;
    case KindCount:
        return;
    }
    load();
}

} // namespace settings
