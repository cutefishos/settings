// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "ui/view.h"

namespace cutefish::ui {
class Label;
class ScrollView;
}

namespace settings {

struct PageInfo;
class SearchField;
class SidebarList;

// The window's left column: its title, a search field, and the pages by
// category. Typing lists matching pages, sub pages included, in place of the
// categories; Up and Down move through them, Return opens one, Escape clears
// the search.
class Sidebar : public cutefish::ui::View
{
public:
    Sidebar();
    ~Sidebar() override;

    // The sidebar pages shown, in order.
    void setEntries(std::vector<const PageInfo*> entries);
    // Takes its text again after the session language changed.
    void retranslate();
    const std::vector<const PageInfo*>& entries() const { return m_entries; }
    // Sub pages whose entry is unavailable stay out of search results.
    void setSubPageAvailable(const std::string& id, bool available);

    // The page shown, and the sidebar page it belongs to.
    void setCurrent(const std::string& page, const std::string& rootPage);

    void focusSearch();
    bool isSearching() const;
    // A press at this point of the window, outside the search field, takes
    // the focus off it.
    void releaseSearch(cutefish::ui::PointF windowPosition);

    // The title's text, laid out as the header's title lines up with it.
    cutefish::ui::Label* titleLabel() const { return m_title; }

    // A click on a sidebar page, or a search result opened.
    std::function<void(const std::string& page, const std::string& rootPage)> onActivated;

    cutefish::ui::SizeF sizeHint() const override;

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;
    bool keyPress(const cutefish::ui::KeyEvent& event) override;

private:
    struct Result
    {
        const PageInfo* page;
        const PageInfo* rootPage;
    };

    void updateResults();
    void openResult(int index);

    std::vector<const PageInfo*> m_entries;
    std::vector<std::string> m_unavailableSubPages;
    std::vector<Result> m_results;
    int m_currentResult = -1;
    std::string m_currentPage;
    std::string m_currentRootPage;
    cutefish::ui::Label* m_title;
    SearchField* m_search;
    cutefish::ui::ScrollView* m_listScroll;
    SidebarList* m_list;
    cutefish::ui::ScrollView* m_resultsScroll;
    SidebarList* m_resultsList;
    cutefish::ui::Label* m_noResults;
};

} // namespace settings
