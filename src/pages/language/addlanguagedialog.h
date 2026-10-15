// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

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

class Language;
class SearchField;

// The catalog of languages not yet preferred, searched as it is typed; the
// chosen one is added at the end of the preferred languages.
class AddLanguageDialog : public FormDialog
{
public:
    AddLanguageDialog(SettingsWindow& parent, Language& language);

private:
    void updateRows();
    void updateStates();
    void select(const std::string& code);

    Language& m_language;
    std::vector<cutefish::base::Subscription> m_subscriptions;
    cutefish::base::Timer m_searchTimer;
    SearchField* m_search;
    cutefish::ui::ScrollView* m_scroll;
    cutefish::ui::ColumnLayout* m_rows;
    cutefish::ui::Label* m_empty;
    cutefish::ui::Label* m_error;
    cutefish::ui::DialogButton* m_cancel;
    cutefish::ui::DialogButton* m_add;
    std::string m_selectedCode;
    bool m_adding = false;
};

} // namespace settings
