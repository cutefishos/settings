// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/defaultapps/defaultapplications.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>

#include "base/application_bundle.h"
#include "base/application_registry.h"
#include "base/mime_apps.h"
#include "base/mime_database.h"

namespace fs = std::filesystem;
using namespace cutefish::base;

namespace settings {

static bool contains(const std::vector<std::string>& list, const std::string& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

// Applications differ in the types they list; the commonest one stands for
// all. Without a choice, this is what the file manager opens them with.
static const char* const kTextType = "text/plain";
static const char* const kMusicType = "audio/mpeg";
static const char* const kVideoType = "video/mp4";
static const char* const kImageType = "image/jpeg";

// Text formats that are documents of another kind, opened by the programs
// for that kind: web pages, calendars, contacts, tables, rich text, printed
// pages, and launchers the file manager runs.
static const char* const kNotForEditing[] = {
    "text/html", "application/xhtml+xml", "text/calendar", "text/vcard", "text/x-vcard", "text/csv",
    "text/tab-separated-values", "text/rtf", "application/rtf", "application/postscript", "application/x-desktop",
};

static bool opensType(const std::vector<std::string>& opened, const std::string& type)
{
    return std::any_of(opened.begin(), opened.end(), [&type](const std::string& listed) {
        if (listed.ends_with("/*"))
            return type.starts_with(std::string_view(listed).substr(0, listed.size() - 1));
        return mime::inherits(type, listed);
    });
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
        Application application{entry->path, mimeapps::listedId(*entry), entry->name, entry->icon,
                                entry->mimeTypes};
        // A bundle has no categories but the terminal's. Any type of a kind
        // is too wide for it (an editor that opens SVG is no image viewer),
        // so it has to open the type the default is read from.
        if (entry->bundle) {
            if (bundle::opensType(*entry, kTextType))
                m_lists[TextEditor].push_back(application);
            if (bundle::opensType(*entry, kMusicType))
                m_lists[MusicPlayer].push_back(application);
            if (bundle::opensType(*entry, kVideoType))
                m_lists[VideoPlayer].push_back(application);
            if (bundle::opensType(*entry, kImageType))
                m_lists[ImageViewer].push_back(application);
            if (contains(entry->categories, "TerminalEmulator"))
                m_lists[Terminal].push_back(application);
            continue;
        }
        // A player can be both the music and the video player.
        if (contains(entry->categories, "WebBrowser") && contains(entry->mimeTypes, "x-scheme-handler/http"))
            m_lists[Browser].push_back(application);
        if (contains(entry->categories, "Email") && contains(entry->mimeTypes, "x-scheme-handler/mailto"))
            m_lists[Email].push_back(application);
        if (contains(entry->categories, "TextEditor") && contains(entry->mimeTypes, kTextType))
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
    const std::string textEditor = mimeapps::defaultApplication(mimeapps::listPaths(), kTextType);
    const std::string musicPlayer = mimeapps::defaultApplication(mimeapps::listPaths(), kMusicType);
    const std::string videoPlayer = mimeapps::defaultApplication(mimeapps::listPaths(), kVideoType);
    const std::string imageViewer = mimeapps::defaultApplication(mimeapps::listPaths(), kImageType);

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

// What the text editor opens: plain text and every kind of it the text
// editors say they edit (logs, scripts, source, markup) that this one can
// open, except media, documents of another kind, and types the lists give an
// application that is no text editor.
std::vector<std::string> DefaultApplications::textTypes(const Application& editor) const
{
    std::vector<std::string> types = {kTextType};
    const std::vector<std::string> paths = mimeapps::listPaths();
    const auto& editors = m_lists[TextEditor];
    for (const Application& other : editors) {
        for (const std::string& listed : other.mimeTypes) {
            const std::string type = mime::canonicalName(listed);
            if (type.empty() || contains(types, type) || !mime::inherits(type, kTextType))
                continue;
            if (type.starts_with("image/") || type.starts_with("audio/") || type.starts_with("video/")
                || std::find(std::begin(kNotForEditing), std::end(kNotForEditing), type) != std::end(kNotForEditing))
                continue;
            if (!opensType(editor.mimeTypes, type))
                continue;
            const std::string chosen = mimeapps::chosenApplication(paths, type);
            const bool chosenElsewhere = !chosen.empty()
                && std::none_of(editors.begin(), editors.end(),
                                [&chosen](const Application& other) { return other.path == chosen; });
            if (!chosenElsewhere)
                types.push_back(type);
        }
    }
    return types;
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
    // A bundle can open a whole group such as image/*, which stands for the
    // types the other choices open and the one the default is read from.
    const auto setAll = [&set, &application, &list](const std::string& prefix, const std::string& common) {
        std::vector<std::string> types;
        const auto add = [&types, &prefix](const std::string& type) {
            if (type.starts_with(prefix) && !type.ends_with("/*") && !contains(types, type))
                types.push_back(type);
        };
        for (const std::string& type : application.mimeTypes)
            add(type);
        if (contains(application.mimeTypes, prefix + "*")) {
            add(common);
            for (const Application& other : list) {
                for (const std::string& type : other.mimeTypes)
                    add(type);
            }
        }
        for (const std::string& type : types)
            set(type, application.fileName);
    };
    switch (kind) {
    case Browser:
        set("x-scheme-handler/http", application.fileName);
        set("x-scheme-handler/https", application.fileName);
        break;
    case Email:
        set("x-scheme-handler/mailto", application.fileName);
        break;
    case TextEditor:
        for (const std::string& type : textTypes(application))
            set(type, application.fileName);
        break;
    case MusicPlayer:
        setAll("audio/", kMusicType);
        break;
    case VideoPlayer:
        setAll("video/", kVideoType);
        break;
    case ImageViewer:
        setAll("image/", kImageType);
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
