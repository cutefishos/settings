// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include <memory>
#include <unistd.h>

#include "accounts/accountsmanager.h"
#include "base/event_loop.h"
#include "base/paths.h"
#include "pages/accounts/password.h"
#include "pages/accounts/usercard.h"
#include "pages/accounts/userdialogs.h"
#include "ui/dialog_window.h"
#include "ui/file_chooser.h"
#include "ui/quick_layout.h"
#include "widgets/metrics.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;

namespace settings {

namespace {

class AccountsPage : public Page
{
public:
    explicit AccountsPage(SettingsWindow& window)
        : Page(window, "Users")
    {
        m_users = content()->addChild<ColumnLayout>();
        m_users->setSpacing(metrics::sectionSpacing);
        m_users->setLayoutHints({.fillWidth = true});
        StandardButton* add = content()->addChild<StandardButton>("Add user");
        add->onClicked = [this] {
            m_addUser = std::make_unique<AddUserDialog>(settingsWindow(), m_manager);
            m_addUser->onClosed = [this] { dispose(m_addUser); };
            m_addUser->show();
        };

        m_accountsChanged = m_manager.accountsChanged.connect([this] { updateUsers(); });
        updateUsers();
    }

private:
    // Lets a dialog finish the event that closed it before it goes.
    template<typename T>
    static void dispose(std::unique_ptr<T>& dialog)
    {
        std::shared_ptr<T> closing(dialog.release());
        base::EventLoop::current()->post([closing] {});
    }

    void updateUsers()
    {
        // Cards that are still there keep their state, such as being unfolded.
        std::vector<std::unique_ptr<View>> previous;
        while (!m_users->children().empty())
            previous.push_back(m_users->takeChild(m_users->children().front().get()));
        for (const auto& account : m_manager.accounts()) {
            std::unique_ptr<View> card;
            for (auto& view : previous) {
                if (view && static_cast<UserCard*>(view.get())->account() == account) {
                    card = std::move(view);
                    break;
                }
            }
            if (!card)
                card = makeCard(account);
            m_users->addChild(std::move(card));
        }
    }

    std::unique_ptr<View> makeCard(const accounts::AccountsManager::AccountPtr& account)
    {
        auto card = std::make_unique<UserCard>(account, int64_t(getuid()));
        card->onPictureClicked = [this, account] {
            m_chooser = std::make_unique<FileChooser>(&settingsWindow().window());
            m_chooser->setMode(FileChooser::Mode::OpenFile);
            // As QML's FileDialog, window-modal.
            m_chooser->setModal(true);
            m_chooser->setFilters(
                    {FileChooser::Filter::fromNameFilter(
                             "Image files (*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.avif)"),
                     FileChooser::Filter::fromNameFilter("All files (*)")});
            m_chooser->setCurrentFolder(base::paths::userDirectory(base::paths::UserDirectory::Pictures));
            m_chooser->onAccepted = [this, account](const std::vector<std::string>& paths, const std::string&) {
                if (paths.empty())
                    return;
                account->setIconFileName(paths.front());
                // Shown from the file picked while the service copies it.
                for (const auto& child : m_users->children()) {
                    auto* card = static_cast<UserCard*>(child.get());
                    if (card->account() == account)
                        card->showPicture(paths.front());
                }
            };
            m_chooser->open();
        };
        card->onChangePassword = [this, account] {
            m_changePassword = std::make_unique<ChangePasswordDialog>(settingsWindow(), account->userName());
            m_changePassword->onPasswordSubmitted = [account](const std::string& password) {
                account->setPassword(cryptPassword(password));
            };
            m_changePassword->onClosed = [this] { dispose(m_changePassword); };
            m_changePassword->show();
        };
        card->onDelete = [this, account] {
            const int64_t uid = account->userId();
            if (uid == int64_t(getuid()))
                return;
            m_confirm = std::make_unique<ConfirmDialog>(
                    &settingsWindow().window(), "Delete this user",
                    "Delete user “" + account->userName()
                            + "”? Their home directory and its contents will be permanently deleted. This "
                              "cannot be undone.");
            m_confirm->setCancelText("Cancel");
            m_confirm->setConfirmText("Delete");
            m_confirm->setDestructive(true);
            m_confirm->onAccepted = [this, uid] { m_manager.deleteUser(uid, true); };
            m_confirm->show();
        };
        return card;
    }

    accounts::AccountsManager m_manager;
    base::Subscription m_accountsChanged;
    ColumnLayout* m_users;
    std::unique_ptr<AddUserDialog> m_addUser;
    std::unique_ptr<ChangePasswordDialog> m_changePassword;
    std::unique_ptr<ConfirmDialog> m_confirm;
    std::unique_ptr<FileChooser> m_chooser;
};

} // namespace

static const PageRegistration registration("accounts", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<AccountsPage>(window);
});

} // namespace settings
