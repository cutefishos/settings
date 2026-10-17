// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "pages/accounts/passwordchanger.h"
#include "widgets/formdialog.h"

namespace cutefish::accounts {
class AccountsManager;
}

namespace cutefish::ui {
class ComboBox;
class DialogButton;
class GridLayout;
class Label;
class TextField;
}

namespace settings {

// A new password for an account, typed twice. With a changer it is the
// user's own: the current password is asked too, and the dialog stays open
// until the change is made, saying why it was not.
class ChangePasswordDialog : public FormDialog
{
public:
    ChangePasswordDialog(SettingsWindow& parent, const std::string& userName,
                         std::unique_ptr<PasswordChanger> changer = nullptr);
    ~ChangePasswordDialog() override;

    // Without a changer: the password as typed; the caller hashes it.
    std::function<void(const std::string& password)> onPasswordSubmitted;

private:
    void update();
    void change();
    void showError(const std::string& error);

    std::unique_ptr<PasswordChanger> m_changer;
    cutefish::ui::GridLayout* m_form;
    cutefish::ui::TextField* m_current = nullptr;
    cutefish::ui::TextField* m_password;
    cutefish::ui::TextField* m_verify;
    cutefish::ui::Label* m_error = nullptr;
    cutefish::ui::DialogButton* m_cancel;
    cutefish::ui::DialogButton* m_change;
};

// A new account: its name, password and type. Stays open, and refuses to
// close, until the service has answered.
class AddUserDialog : public FormDialog
{
public:
    AddUserDialog(SettingsWindow& parent, cutefish::accounts::AccountsManager& manager);

private:
    void update();
    void add();
    void setWaiting(bool waiting);

    cutefish::accounts::AccountsManager& m_manager;
    cutefish::ui::GridLayout* m_form;
    cutefish::ui::TextField* m_name;
    cutefish::ui::TextField* m_password;
    cutefish::ui::TextField* m_verify;
    cutefish::ui::ComboBox* m_type;
    cutefish::ui::Label* m_error;
    cutefish::ui::DialogButton* m_cancel;
    cutefish::ui::DialogButton* m_add;
    bool m_waiting = false;
};

} // namespace settings
