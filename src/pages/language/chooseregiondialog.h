// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>

#include "base/signal.h"
#include "base/timer.h"
#include "widgets/formdialog.h"

namespace cutefish::ui {
class ColumnLayout;
class DialogButton;
class Label;
class ScrollView;
}

namespace settings {

class Region;
class SearchField;

// Every region, searched as it is typed, the current one checked; Choose
// makes the checked one the session's region.
class ChooseRegionDialog : public FormDialog
{
public:
    ChooseRegionDialog(SettingsWindow& parent, Region& region);

private:
    void updateRows();
    void updateStates();
    void select(const std::string& code);

    Region& m_region;
    std::vector<cutefish::base::Subscription> m_subscriptions;
    cutefish::base::Timer m_searchTimer;
    SearchField* m_search;
    cutefish::ui::ScrollView* m_scroll;
    cutefish::ui::ColumnLayout* m_rows;
    cutefish::ui::Label* m_empty;
    cutefish::ui::DialogButton* m_choose;
    std::string m_selectedCode;
    bool m_revealSelection = true;
};

} // namespace settings
