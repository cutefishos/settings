// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/defaultapps/defaultapplications.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <optional>

#include "base/application_registry.h"
#include "base/file.h"
#include "base/locale.h"
#include "base/mime_apps.h"
#include "base/mime_database.h"
#include "base/paths.h"
#include "base/settings.h"

namespace fs = std::filesystem;
using namespace cutefish::base;

namespace settings {

static bool contains(const std::vector<std::string>& list, const std::string& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

// Hidden entries only mask a system entry; programs that need a terminal are
// not offered for opening files (as the file manager has it).
static bool opensFiles(const DesktopEntry& entry)
{
    return !entry.hidden && !entry.terminal;
}

// The language the desktop shows itself in, as "de_DE".
static std::string sessionLanguage()
{
    const Settings settings(paths::cutefishConfig("services"));
    std::string language = settings.stringValue("Language/language");
    if (language.empty())
        return Locale::systemLanguage();
    std::replace(language.begin(), language.end(), '-', '_');
    return language;
}

// The text of the first <comment> element with the attribute, or without
// one for an empty attribute; nothing when the file has none.
static std::optional<std::string> commentIn(const std::string& xml, const std::string& attribute)
{
    size_t at = 0;
    while ((at = xml.find("<comment", at)) != std::string::npos) {
        const size_t close = xml.find('>', at);
        if (close == std::string::npos)
            return std::nullopt;
        const std::string tag = xml.substr(at + 8, close - at - 8);
        const bool localized = tag.find("xml:lang=") != std::string::npos;
        const bool wanted = attribute.empty() ? !localized : tag.find("xml:lang=\"" + attribute + "\"") != std::string::npos;
        if (wanted) {
            const size_t end = xml.find("</comment>", close);
            if (end == std::string::npos)
                return std::nullopt;
            return xml.substr(close + 1, end - close - 1);
        }
        at = close;
    }
    return std::nullopt;
}

// The shared MIME database's description of a type in the session language,
// with the usual fallbacks from "de_DE" to "de" and then the untranslated one.
static std::string typeDescription(const std::string& type, const std::vector<std::string>& languages)
{
    std::vector<std::string> directories = {paths::dataHome()};
    for (const std::string& directory : paths::dataDirectories())
        directories.push_back(directory);
    for (const std::string& directory : directories) {
        const std::optional<std::string> xml = file::read(directory + "/mime/" + type + ".xml");
        if (!xml)
            continue;
        for (const std::string& language : languages) {
            if (std::optional<std::string> comment = commentIn(*xml, language))
                return *comment;
        }
        if (std::optional<std::string> comment = commentIn(*xml, std::string()))
            return *comment;
    }
    return type;
}

static bool playsVideo(const DesktopEntry& entry)
{
    if (!contains(entry.categories, "AudioVideo"))
        return false;
    return std::any_of(entry.mimeTypes.begin(), entry.mimeTypes.end(),
                       [](const std::string& type) { return type.starts_with("video/"); });
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
        if (contains(entry->categories, "WebBrowser") && contains(entry->mimeTypes, "x-scheme-handler/http"))
            m_lists[Browser].push_back(std::move(application));
        else if (contains(entry->categories, "Email") && contains(entry->mimeTypes, "x-scheme-handler/mailto"))
            m_lists[Email].push_back(std::move(application));
        else if (contains(entry->categories, "TerminalEmulator"))
            m_lists[Terminal].push_back(std::move(application));
        else if (playsVideo(*entry))
            m_lists[VideoPlayer].push_back(std::move(application));
    }

    const auto first = [](const std::vector<std::string>& list) { return list.empty() ? std::string() : list.front(); };
    const std::string browser = first(mimeapps::associations(mimeapps::userListPath(), "Default Applications",
                                                             "x-scheme-handler/http"));
    const std::string email = first(mimeapps::associations(mimeapps::userListPath(), "Default Applications",
                                                           "x-scheme-handler/mailto"));
    const std::string terminal = mimeapps::defaultTerminal(mimeapps::userTerminalsListPath());
    // Players differ in the video types they list; the commonest one stands
    // for all. Without a choice, this is what the file manager opens videos with.
    const std::string videoPlayer = mimeapps::defaultApplication(mimeapps::listPaths(), "video/mp4");

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
    find(Terminal, terminal, false);
    find(VideoPlayer, videoPlayer, true);
    loadFileTypes();
    changed.emit();
}

// Every type some installed application opens, scheme handlers and
// folders aside: those are the rows above, or not a file.
void DefaultApplications::loadFileTypes()
{
    m_fileTypes.clear();
    ApplicationRegistry& registry = ApplicationRegistry::instance();
    std::vector<std::string> types;
    for (const auto& entry : registry.applications()) {
        if (!opensFiles(*entry))
            continue;
        for (const std::string& name : entry->mimeTypes) {
            const std::string type = mime::canonicalName(name);
            if (type.empty() || type.starts_with("x-scheme-handler/") || type.starts_with("inode/")
                || contains(types, type))
                continue;
            types.push_back(type);
        }
    }

    // "de_DE@latin" is looked up as itself, then "de_DE", "de@latin", "de", "de-DE".
    const std::string language = sessionLanguage();
    const size_t modifierStart = language.find('@');
    const std::string modifier = modifierStart == std::string::npos ? std::string() : language.substr(modifierStart);
    const std::string name = language.substr(0, modifierStart);
    const std::string base = name.substr(0, name.find('_'));
    std::string bcp47 = name;
    std::replace(bcp47.begin(), bcp47.end(), '_', '-');
    std::vector<std::string> languages;
    for (const std::string& candidate : {name + modifier, name, base + modifier, base, bcp47}) {
        if (!candidate.empty() && !contains(languages, candidate))
            languages.push_back(candidate);
    }

    const std::vector<std::string> lists = mimeapps::listPaths();
    for (const std::string& type : types) {
        FileType fileType{type, typeDescription(type, languages), mime::iconName(type), {}, -1};
        const std::string chosen = mimeapps::defaultApplication(lists, type);
        for (const std::string& path : mimeapps::applications(type, lists)) {
            const auto entry = registry.byPath(path);
            if (!entry)
                continue;
            if (path == chosen)
                fileType.defaultIndex = int(fileType.applications.size());
            fileType.applications.push_back({entry->path, fs::path(entry->path).filename().string(), entry->name,
                                             entry->icon, entry->mimeTypes});
        }
        if (!fileType.applications.empty())
            m_fileTypes.push_back(std::move(fileType));
    }
    const Locale& locale = Locale::system();
    std::stable_sort(m_fileTypes.begin(), m_fileTypes.end(), [&locale](const FileType& a, const FileType& b) {
        return locale.compare(a.description, b.description) < 0;
    });
    loadGroups();
}

// The group a type belongs to, by the MIME database's generic icon for it:
// office formats are documents whatever their media type, say.
static size_t groupFor(const std::string& type, const std::vector<DefaultApplications::FileTypeGroup>& groups)
{
    const std::string generic = mime::genericIconName(type);
    const auto index = [&groups](const char* icon) {
        for (size_t i = 0; i < groups.size(); ++i) {
            if (groups[i].icon == icon)
                return i;
        }
        return groups.size() - 1;
    };
    if (generic == "image-x-generic")
        return index("image-x-generic");
    if (generic == "audio-x-generic")
        return index("audio-x-generic");
    if (generic == "video-x-generic")
        return index("video-x-generic");
    if (generic == "text-x-generic" || generic == "text-x-script" || generic == "text-html"
        || generic == "text-x-generic-template")
        return index("text-x-generic");
    if (generic.starts_with("x-office-"))
        return index("x-office-document");
    if (generic == "package-x-generic")
        return index("package-x-generic");
    if (generic == "font-x-generic")
        return index("font-x-generic");
    return groups.size() - 1;
}

void DefaultApplications::loadGroups()
{
    m_groups = {{"Images", "image-x-generic"},   {"Audio", "audio-x-generic"},     {"Video", "video-x-generic"},
                {"Text", "text-x-generic"},      {"Documents", "x-office-document"}, {"Archives", "package-x-generic"},
                {"Fonts", "font-x-generic"},     {"Other", "application-x-generic"}};
    for (size_t i = 0; i < m_fileTypes.size(); ++i)
        m_groups[groupFor(m_fileTypes[i].mimeType, m_groups)].types.push_back(i);
    std::erase_if(m_groups, [](const FileTypeGroup& group) { return group.types.empty(); });

    for (FileTypeGroup& group : m_groups) {
        // Each application with how many of the group's types it opens, and
        // of how many it is the default.
        struct Candidate
        {
            const Application* application;
            size_t opens = 0;
            size_t defaults = 0;
        };
        std::vector<Candidate> candidates;
        for (size_t typeIndex : group.types) {
            const FileType& fileType = m_fileTypes[typeIndex];
            for (size_t i = 0; i < fileType.applications.size(); ++i) {
                const Application& application = fileType.applications[i];
                auto found = std::find_if(candidates.begin(), candidates.end(), [&application](const Candidate& c) {
                    return c.application->path == application.path;
                });
                if (found == candidates.end())
                    found = candidates.insert(candidates.end(), {&application});
                ++found->opens;
                if (int(i) == fileType.defaultIndex)
                    ++found->defaults;
            }
        }
        const Locale& locale = Locale::system();
        std::stable_sort(candidates.begin(), candidates.end(), [&locale](const Candidate& a, const Candidate& b) {
            if (a.opens != b.opens)
                return a.opens > b.opens;
            return locale.compare(a.application->name, b.application->name) < 0;
        });
        // The group's default: the one that is the default of most of its types.
        size_t best = 0;
        for (size_t i = 0; i < candidates.size(); ++i) {
            group.applications.push_back(*candidates[i].application);
            if (candidates[i].defaults > best) {
                best = candidates[i].defaults;
                group.defaultIndex = int(i);
            }
        }
    }
}

void DefaultApplications::setGroupDefault(size_t groupIndex, int applicationIndex)
{
    if (groupIndex >= m_groups.size())
        return;
    const FileTypeGroup& group = m_groups[groupIndex];
    if (applicationIndex < 0 || applicationIndex >= int(group.applications.size()))
        return;
    const std::string& path = group.applications[size_t(applicationIndex)].path;
    const auto entry = ApplicationRegistry::instance().byPath(path);
    if (!entry)
        return;
    const std::string desktopId = mimeapps::listedId(*entry);
    const std::string userList = mimeapps::userListPath();
    for (size_t typeIndex : group.types) {
        const FileType& fileType = m_fileTypes[typeIndex];
        const bool opens = std::any_of(fileType.applications.begin(), fileType.applications.end(),
                                       [&path](const Application& a) { return a.path == path; });
        if (opens && !mimeapps::setDefault(userList, fileType.mimeType, desktopId))
            fprintf(stderr, "Failed to save default application for %s\n", fileType.mimeType.c_str());
    }
    load();
}

void DefaultApplications::setFileTypeDefault(size_t typeIndex, int applicationIndex)
{
    if (typeIndex >= m_fileTypes.size())
        return;
    const FileType& fileType = m_fileTypes[typeIndex];
    if (applicationIndex < 0 || applicationIndex >= int(fileType.applications.size()))
        return;
    // The list names a bundle by its path, an installed program by its desktop file.
    const auto entry = ApplicationRegistry::instance().byPath(fileType.applications[size_t(applicationIndex)].path);
    if (!entry)
        return;
    if (!mimeapps::setDefault(mimeapps::userListPath(), fileType.mimeType, mimeapps::listedId(*entry))) {
        fprintf(stderr, "Failed to save default application for %s\n", fileType.mimeType.c_str());
        return;
    }
    load();
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
    switch (kind) {
    case Browser:
        set("x-scheme-handler/http", application.fileName);
        set("x-scheme-handler/https", application.fileName);
        break;
    case Email:
        set("x-scheme-handler/mailto", application.fileName);
        break;
    case Terminal:
        if (!mimeapps::setDefaultTerminal(mimeapps::userTerminalsListPath(), application.fileName)) {
            fprintf(stderr, "Failed to save the default terminal\n");
            return;
        }
        break;
    case VideoPlayer:
        for (const std::string& type : application.mimeTypes) {
            if (type.starts_with("video/"))
                set(type, application.fileName);
        }
        break;
    case KindCount:
        return;
    }
    load();
}

} // namespace settings
