// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <array>
#include <memory>
#include <unicode/ucasemap.h>

#include "base/i18n.h"
#include "base/mime_database.h"
#include "pages/defaultapps/defaultapplications.h"
#include "ui/icon.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowcombobox.h"
#include "widgets/searchfield.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish::ui;
using cutefish::i18n::tr;

namespace settings {

namespace {

// Unicode's default lower case, as JavaScript's toLowerCase() gives it.
std::string lowerCase(const std::string& text)
{
    UErrorCode status = U_ZERO_ERROR;
    std::unique_ptr<UCaseMap, decltype(&ucasemap_close)> map(ucasemap_open("", 0, &status), ucasemap_close);
    if (U_FAILURE(status))
        return text;
    std::string result(text.size() + 16, '\0');
    int32_t length = ucasemap_utf8ToLower(map.get(), result.data(), int32_t(result.size()), text.data(),
                                          int32_t(text.size()), &status);
    if (status == U_BUFFER_OVERFLOW_ERROR) {
        status = U_ZERO_ERROR;
        result.resize(size_t(length));
        length = ucasemap_utf8ToLower(map.get(), result.data(), int32_t(result.size()), text.data(),
                                      int32_t(text.size()), &status);
    }
    if (U_FAILURE(status))
        return text;
    result.resize(size_t(length));
    return result;
}

std::string trimmed(const std::string& text)
{
    const auto space = [](unsigned char c) { return c == ' ' || (c >= '\t' && c <= '\r'); };
    size_t start = 0;
    size_t end = text.size();
    while (start < end && space(text[start]))
        ++start;
    while (end > start && space(text[end - 1]))
        --end;
    return text.substr(start, end - start);
}

class DefaultAppsPage : public Page
{
public:
    explicit DefaultAppsPage(SettingsWindow& window)
        : Page(window, tr("Default Applications"))
    {
        Card* card = content()->addChild<Card>(Card::Style::List);
        const std::array<const char*, DefaultApplications::KindCount> titles = {tr("Web browser"), tr("Email client"),
                                                                               tr("Terminal"), tr("Video player")};
        for (int kind = 0; kind < DefaultApplications::KindCount; ++kind) {
            ListRow* row = card->addChild<ListRow>(titles[size_t(kind)]);
            row->setInteractive(false);
            RowComboBox* box = row->addContent<RowComboBox>();
            box->onActivated = [this, kind](int index) {
                m_applications.setDefault(DefaultApplications::Kind(kind), index);
            };
            m_rows[size_t(kind)] = {row, box};
        }

        // Every type of file an installed application opens, in groups that
        // unfold to the types, searchable.
        Section* fileTypes = content()->addChild<Section>(tr("File types"));
        fileTypes->setSpacing(metrics::m);
        m_search = fileTypes->addChild<SearchField>();
        m_search->setName("defaultapps:search");
        m_search->setPlaceholder(tr("Search file types"));
        m_search->onTextChanged = [this] { filter(); };
        m_groupCard = fileTypes->addChild<Card>(Card::Style::List);
        m_noMatch = fileTypes->addChild<Footnote>(tr("No matching file types"));
        m_noMatch->setVisible(false);

        m_subscription = m_applications.changed.connect([this] { update(); });
        update();
    }

private:
    struct TypeRow
    {
        ListRow* row;
        RowComboBox* box;
        // Lower case, for the search.
        std::string description;
        std::string mimeType;
        size_t group;
        // Index into fileTypes().
        size_t type;
        bool matches = true;
    };

    struct GroupRow
    {
        ListRow* row;
        RowComboBox* box;
        TrailingIcon* chevron;
        std::string title;
        std::vector<std::string> mimeTypes;
        // Unfolded by a click; a search unfolds the groups with matches itself.
        bool unfolded = false;
    };

    void update()
    {
        for (int kind = 0; kind < DefaultApplications::KindCount; ++kind) {
            const auto& list = m_applications.applications(DefaultApplications::Kind(kind));
            auto [row, box] = m_rows[size_t(kind)];
            std::vector<RowComboBox::Item> items;
            for (const auto& application : list)
                items.push_back({application.name, application.icon});
            box->setItems(std::move(items));
            const int index = m_applications.defaultIndex(DefaultApplications::Kind(kind));
            box->setCurrentIndex(index >= 0 ? index : (list.empty() ? -1 : 0));
            box->setDisplayText(list.empty() ? std::optional<std::string>(tr("None")) : std::nullopt);
            row->setEnabled(!list.empty());
        }
        updateFileTypes();
    }

    static std::vector<RowComboBox::Item> itemsFor(const std::vector<DefaultApplications::Application>& list)
    {
        std::vector<RowComboBox::Item> items;
        for (const auto& application : list)
            items.push_back({application.name, application.icon});
        return items;
    }

    // The rows are kept across reloads, which every saved choice causes, so
    // the search, the unfolded groups and the scroll position stay; only a
    // changed set of types builds them anew.
    void updateFileTypes()
    {
        const auto& groups = m_applications.fileTypeGroups();
        const auto& fileTypes = m_applications.fileTypes();
        bool same = groups.size() == m_groupRows.size();
        for (size_t g = 0; same && g < groups.size(); ++g) {
            same = groups[g].title == m_groupRows[g].title && groups[g].types.size() == m_groupRows[g].mimeTypes.size();
            for (size_t i = 0; same && i < groups[g].types.size(); ++i)
                same = fileTypes[groups[g].types[i]].mimeType == m_groupRows[g].mimeTypes[i];
        }
        if (!same)
            buildRows();

        for (size_t g = 0; g < groups.size(); ++g) {
            const auto& group = groups[g];
            RowComboBox* box = m_groupRows[g].box;
            box->setItems(itemsFor(group.applications));
            box->setCurrentIndex(group.defaultIndex >= 0 ? group.defaultIndex : 0);
            box->setDisplayText(group.defaultIndex >= 0 ? std::nullopt : std::optional<std::string>(tr("Various")));
        }
        for (TypeRow& typeRow : m_typeRows) {
            const auto& fileType = fileTypes[typeRow.type];
            typeRow.box->setItems(itemsFor(fileType.applications));
            typeRow.box->setCurrentIndex(fileType.defaultIndex >= 0 ? fileType.defaultIndex : 0);
        }
    }

    void buildRows()
    {
        for (GroupRow& groupRow : m_groupRows)
            m_groupCard->takeChild(groupRow.row);
        m_groupRows.clear();
        m_typeRows.clear();

        const auto& groups = m_applications.fileTypeGroups();
        const auto& fileTypes = m_applications.fileTypes();
        for (size_t g = 0; g < groups.size(); ++g) {
            const auto& group = groups[g];
            ListRow* row = m_groupCard->addChild<ListRow>(tr(group.title));
            row->setLeading<ThemeIcon>(group.icon, 24);
            RowComboBox* box = row->addContent<RowComboBox>();
            box->onActivated = [this, g](int index) { m_applications.setGroupDefault(g, index); };
            TrailingIcon* chevron = row->addContent<TrailingIcon>("chevron-right");
            row->onClicked = [this, g] {
                // While searching the matches stay unfolded.
                if (!query().empty())
                    return;
                GroupRow& groupRow = m_groupRows[g];
                groupRow.unfolded = !groupRow.unfolded;
                unfold(groupRow, groupRow.unfolded);
            };
            std::vector<std::string> mimeTypes;
            ColumnLayout* details = row->details();
            details->setPadding({metrics::xl, 0, 0, 0});
            for (size_t typeIndex : group.types) {
                const auto& fileType = fileTypes[typeIndex];
                mimeTypes.push_back(fileType.mimeType);
                ListRow* typeRow = details->addChild<ListRow>(fileType.description);
                typeRow->setInteractive(false);
                typeRow->setContentAbove(true);
                typeRow->setToolTip(fileType.mimeType);
                // The theme may lack the type's own icon; then its group's generic one.
                ThemeIcon* icon = typeRow->setLeading<ThemeIcon>(fileType.icon, 24);
                if (icon->path().empty())
                    icon->setIconName(cutefish::base::mime::genericIconName(fileType.mimeType));
                RowComboBox* typeBox = typeRow->addContent<RowComboBox>();
                typeBox->onActivated = [this, typeIndex](int index) {
                    m_applications.setFileTypeDefault(typeIndex, index);
                };
                m_typeRows.push_back({typeRow, typeBox, lowerCase(fileType.description), fileType.mimeType, g,
                                      typeIndex});
            }
            m_groupRows.push_back({row, box, chevron, group.title, std::move(mimeTypes)});
        }
        filter();
    }

    std::string query() const { return lowerCase(trimmed(m_search->text())); }

    void unfold(GroupRow& groupRow, bool unfolded)
    {
        groupRow.row->setExpanded(unfolded);
        groupRow.chevron->icon()->setIconName(unfolded ? "chevron-down" : "chevron-right");
    }

    void filter()
    {
        const std::string text = query();
        std::vector<int> shown(m_groupRows.size(), 0);
        for (TypeRow& typeRow : m_typeRows) {
            typeRow.matches = text.empty() || typeRow.description.find(text) != std::string::npos
                || typeRow.mimeType.find(text) != std::string::npos;
            typeRow.row->setVisible(typeRow.matches);
            if (typeRow.matches)
                ++shown[typeRow.group];
        }
        // Rows draw their separators by their visible neighbours, so hiding is enough.
        int total = 0;
        for (size_t g = 0; g < m_groupRows.size(); ++g) {
            GroupRow& groupRow = m_groupRows[g];
            groupRow.row->setVisible(shown[g] > 0);
            unfold(groupRow, text.empty() ? groupRow.unfolded : shown[g] > 0);
            total += shown[g];
        }
        m_groupCard->setVisible(total > 0);
        m_noMatch->setVisible(total == 0 && !m_typeRows.empty());
    }

    DefaultApplications m_applications;
    cutefish::base::Subscription m_subscription;
    std::array<std::pair<ListRow*, RowComboBox*>, DefaultApplications::KindCount> m_rows;
    SearchField* m_search;
    Card* m_groupCard;
    std::vector<GroupRow> m_groupRows;
    std::vector<TypeRow> m_typeRows;
    Footnote* m_noMatch;
};

} // namespace

static const PageRegistration registration("defaultapps", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<DefaultAppsPage>(window);
});

} // namespace settings
