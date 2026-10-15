// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "accounts/useraccount.h"
#include "base/signal.h"
#include "widgets/card.h"

namespace cutefish::ui {
class Animation;
class ColumnLayout;
class Label;
}

namespace settings {

class StandardButton;

// The part of a card that unfolds below its header, its height animated.
class Unfold : public cutefish::ui::View
{
public:
    Unfold();
    ~Unfold() override;

    cutefish::ui::ColumnLayout* content() const { return m_content; }
    bool isShown() const { return m_shown; }
    void setShown(bool shown);
    void toggle() { setShown(!m_shown); }

    cutefish::ui::SizeF sizeHint() const override;
    double heightForWidth(double width) const override;

protected:
    void layout() override;

private:
    double targetHeight(double width) const;

    cutefish::ui::ColumnLayout* m_content;
    std::unique_ptr<cutefish::ui::Animation> m_animation;
    bool m_shown = false;
    // The height drawn now, and the one the animation started from.
    double m_height = 0;
    double m_from = 0;
    double m_progress = 1;
};

// One account on the Users page: its picture, names and whether it is the
// one logged in, unfolding to its type and the password and delete buttons.
class UserCard : public Card
{
public:
    UserCard(std::shared_ptr<cutefish::accounts::UserAccount> account, int64_t loggedInUid);
    ~UserCard() override;

    const std::shared_ptr<cutefish::accounts::UserAccount>& account() const { return m_account; }
    // Shows a picture before the account has it.
    void showPicture(const std::string& path);

    std::function<void()> onPictureClicked;
    std::function<void()> onChangePassword;
    std::function<void()> onDelete;

protected:
    void layout() override;

private:
    class Header;

    void update();

    std::shared_ptr<cutefish::accounts::UserAccount> m_account;
    int64_t m_loggedInUid;
    cutefish::base::Subscription m_changed;
    Header* m_header;
    Unfold* m_unfold;
    cutefish::ui::Label* m_accountType;
    StandardButton* m_password;
    StandardButton* m_delete;
};

} // namespace settings
