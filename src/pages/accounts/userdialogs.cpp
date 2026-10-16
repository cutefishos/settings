// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/accounts/userdialogs.h"

#include "accounts/accountsmanager.h"
#include "pages/accounts/password.h"
#include "ui/combo_box.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/metrics.h"

using namespace cutefish;
using namespace cutefish::ui;

namespace settings {

static constexpr double kWidth = 400;

static GridLayout* addForm(ColumnLayout* content)
{
    GridLayout* grid = content->addChild<GridLayout>(2);
    grid->setColumnSpacing(metrics::dialogSpacing);
    grid->setRowSpacing(metrics::formRowSpacing);
    grid->setLayoutHints({.fillWidth = true});
    return grid;
}

static TextField* addField(GridLayout* grid, const std::string& label, bool password)
{
    grid->addChild<Label>(label);
    TextField* field = grid->addChild<TextField>();
    field->setPlaceholder(label);
    if (password)
        field->setEchoMode(TextField::EchoMode::Password);
    field->setLayoutHints({.fillWidth = true});
    return field;
}

// Secondary text that wraps, recoloured with the theme.
class SecondaryLabel : public Label
{
public:
    using Label::Label;

protected:
    void layout() override
    {
        setColor(Theme::current().disabledText);
        Label::layout();
    }
};

ChangePasswordDialog::ChangePasswordDialog(SettingsWindow& parent, const std::string& userName)
    : FormDialog(parent, "Change password", kWidth)
{
    Label* name = content()->addChild<SecondaryLabel>(userName);
    name->setWrap(true);
    name->setLayoutHints({.fillWidth = true});

    GridLayout* grid = addForm(content());
    m_password = addField(grid, "Password", true);
    m_verify = addField(grid, "Verify password", true);

    RowLayout* buttons = addButtonRow();
    DialogButton* cancel = buttons->addChild<DialogButton>("Cancel");
    cancel->onClicked = [this] { close(); };
    m_change = buttons->addChild<DialogButton>("Change password", true);
    m_change->onClicked = [this] {
        const std::string password = m_password->text();
        if (onPasswordSubmitted)
            onPasswordSubmitted(password);
        close();
    };

    m_password->onTextChanged = [this] { update(); };
    m_verify->onTextChanged = [this] { update(); };
    onShown = [this] { m_password->setFocus(); };
    update();
}

void ChangePasswordDialog::update()
{
    m_change->setEnabled(!m_password->text().empty() && m_password->text() == m_verify->text());
}

AddUserDialog::AddUserDialog(SettingsWindow& parent, accounts::AccountsManager& manager)
    : FormDialog(parent, "Add new user", kWidth)
    , m_manager(manager)
{
    m_form = addForm(content());
    m_name = addField(m_form, "User name", false);
    m_password = addField(m_form, "Password", true);
    m_verify = addField(m_form, "Verify password", true);
    m_form->addChild<Label>("Account type");
    m_type = m_form->addChild<ComboBox>(std::vector<std::string>{"Standard", "Administrator"});
    m_type->setCurrentIndex(0);
    m_type->setLayoutHints({.fillWidth = true});

    m_error = content()->addChild<Label>();
    m_error->setWrap(true);
    m_error->setLayoutHints({.fillWidth = true});
    m_error->setVisible(false);

    RowLayout* buttons = addButtonRow();
    m_cancel = buttons->addChild<DialogButton>("Cancel");
    m_cancel->onClicked = [this] { requestClose(); };
    m_add = buttons->addChild<DialogButton>("Add", true);
    m_add->onClicked = [this] { add(); };

    for (TextField* field : {m_name, m_password, m_verify})
        field->onTextChanged = [this] { update(); };
    canClose = [this] { return !m_waiting; };
    onShown = [this] { m_name->setFocus(); };
    update();
}

void AddUserDialog::update()
{
    m_form->setEnabled(!m_waiting);
    m_cancel->setEnabled(!m_waiting);
    m_add->setEnabled(!m_waiting && !m_name->text().empty() && !m_password->text().empty()
                      && m_password->text() == m_verify->text());
}

void AddUserDialog::setWaiting(bool waiting)
{
    m_waiting = waiting;
    update();
}

void AddUserDialog::add()
{
    m_error->setVisible(false);
    m_error->setText({});
    const std::string password = m_password->text();
    setWaiting(true);
    m_manager.createUser(m_name->text(), {},
                         m_type->currentIndex() == 1 ? accounts::UserAccount::Administrator
                                                     : accounts::UserAccount::Standard,
                         [this, password](const accounts::AccountsManager::AccountPtr& account,
                                          const std::string& error) {
                             setWaiting(false);
                             if (!account) {
                                 m_error->setText(error);
                                 m_error->setVisible(!error.empty());
                                 return;
                             }
                             account->setPasswordMode(accounts::UserAccount::RegularPassword);
                             account->setPassword(cryptPassword(password));
                             close();
                         });
}

} // namespace settings
