// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/accounts/usercard.h"

#include <algorithm>
#include <chrono>
#include <cmath>

#include <unicode/unistr.h>
#include <unicode/utf16.h>

#include "ui/animation.h"
#include "ui/color_provider.h"
#include "ui/icon.h"
#include "ui/image_view.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "ui/cell_layout.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/colors.h"
#include "widgets/metrics.h"
#include "widgets/standardbutton.h"

using namespace cutefish;
using namespace cutefish::ui;

namespace settings {

namespace {

// FishUI's RoundImageButton with a glyph: a round plate on hover and press,
// kept on the glyph's pixel parity so both share a centre.
class DisclosureButton : public View
{
public:
    DisclosureButton(double size, double iconMargins)
        : m_size(size)
        , m_iconSize(std::round(size - iconMargins * 2))
    {
        m_icon = addChild<Icon>(std::string(), m_iconSize);
    }

    void setIconName(std::string name) { m_icon->setIconName(std::move(name)); }
    const std::string& iconName() const { return m_icon->iconName(); }
    double iconSize() const { return m_iconSize; }

    SizeF sizeHint() const override { return {m_size, m_size}; }
    bool acceptsPointer() const override { return true; }

    std::function<void()> onClicked;

protected:
    void paint(Painter& painter) override
    {
        const Rect box = painter.deviceRect(localRect());
        const int icon = int(std::lround(m_iconSize * painter.scale()));
        if (isEnabled() && (m_hovered || m_pressed)) {
            const int margin = int(std::lround(std::round(m_size * 0.1) * painter.scale()));
            int plate = std::min(box.width, box.height) - 2 * margin;
            if ((plate - icon) % 2 != 0)
                plate -= 1;
            painter.fillDeviceRoundedRect({box.x + (box.width - plate) / 2, box.y + (box.height - plate) / 2, plate,
                                           plate},
                                          plate / 2.0,
                                          color(m_pressed ? ColorId::RoundButtonPressed : ColorId::RoundButtonHovered));
        }
    }

    void layout() override
    {
        m_icon->setGeometry({std::round((m_size - m_iconSize) / 2), std::round((m_size - m_iconSize) / 2), m_iconSize,
                             m_iconSize});
    }

    void pointerEnter() override
    {
        m_hovered = true;
        requestPaint();
    }
    void pointerLeave() override
    {
        m_hovered = false;
        requestPaint();
    }
    void pointerPress(const PointerEvent& event) override
    {
        if (event.button != MouseButton::Left)
            return;
        m_pressed = true;
        requestPaint();
    }
    void pointerRelease(const PointerEvent& event) override
    {
        const bool clicked = m_pressed && localRect().contains(event.position);
        m_pressed = false;
        requestPaint();
        if (clicked && onClicked)
            onClicked();
    }

private:
    double m_size;
    double m_iconSize;
    Icon* m_icon;
    bool m_hovered = false;
    bool m_pressed = false;
};

// The account's picture in a circle, or its initial, as on the lock screen,
// for an account with no picture: the icon theme has no avatar that size.
class UserAvatar : public View
{
public:
    static constexpr double kSize = 48;

    UserAvatar()
    {
        m_initial = addChild<Label>();
        m_initial->setFontSize(22);
        // Drawn once decoded; the initial shows until then.
        m_picture = addChild<ImageView>();
        m_picture->setCornerRadius(kSize / 2);
        m_picture->onStatusChanged = [this](ImageView::Status) {
            requestLayout();
            requestPaint();
        };
    }

    void setUserName(const std::string& name)
    {
        // The first character, upper case, as QString::charAt(0).toUpper() gives it.
        icu::UnicodeString text = icu::UnicodeString::fromUTF8(name);
        std::string initial = "?";
        if (!text.isEmpty()) {
            icu::UnicodeString first = text.tempSubString(0, text.length() > 0 && U16_IS_LEAD(text[0]) ? 2 : 1);
            first.toUpper();
            initial.clear();
            first.toUTF8String(initial);
        }
        m_initial->setText(initial);
        requestLayout();
    }

    void setPicture(const std::string& path)
    {
        if (path == m_picture->source())
            return;
        m_picture->setSource(path);
        requestLayout();
        requestPaint();
    }

    SizeF sizeHint() const override { return {kSize, kSize}; }
    bool acceptsPointer() const override { return true; }
    CursorShape cursorShape() const override { return CursorShape::Pointer; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        m_initial->setColor(Theme::current().disabledText);
        const SizeF size = m_initial->sizeHint();
        m_initial->setGeometry({rounding::centeredStart(kSize, size.width), rounding::centeredStart(kSize, size.height), size.width, size.height});
        m_initial->setVisible(!hasPicture());
        m_picture->setGeometry(localRect());
    }

    void paint(Painter& painter) override
    {
        // The account's initial, as on the lock screen, for an account with
        // no picture: the icon theme has no avatar the size of a picture.
        if (!hasPicture())
            painter.fillEllipse(localRect(), Theme::current().text.withAlpha(0.12));
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left && localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    bool hasPicture() const { return m_picture->status() == ImageView::Status::Ready; }

    Label* m_initial;
    ImageView* m_picture;
};

} // namespace

// The card's top row: an item as tall as its row plus a margin, holding the
// picture, the user name, the real name, then "Logged in" and the
// disclosure button at the end.
class UserCard::Header : public View
{
public:
    Header()
    {
        row = addChild<RowLayout>();
        row->setSpacing(metrics::rowSpacing);
        avatar = row->addChild<UserAvatar>();
        userName = row->addChild<Label>();
        realName = row->addChild<Label>();
        row->addChild<View>()->setLayoutHints({.fillWidth = true});
        loggedIn = row->addChild<Label>("Logged in");
        disclosure = row->addChild<DisclosureButton>(26, 5);
        disclosure->setIconName("chevron-down");
    }

    SizeF sizeHint() const override { return {0, row->sizeHint().height + metrics::m}; }
    bool acceptsPointer() const override { return true; }

    RowLayout* row;
    UserAvatar* avatar;
    Label* userName;
    Label* realName;
    Label* loggedIn;
    DisclosureButton* disclosure;
    std::function<void()> onDoubleClicked;

protected:
    void pointerPress(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left && event.clickCount == 2 && onDoubleClicked)
            onDoubleClicked();
    }

    void layout() override
    {
        const Theme& theme = Theme::current();
        realName->setColor(theme.disabledText);
        loggedIn->setColor(theme.disabledText);
        // As close to the arrow as a row choice's value is to its arrow.
        loggedIn->setLayoutHints({.margins = {0, 0, theme.mediumSpacing - metrics::rowSpacing - 5, 0}});
        disclosure->setLayoutHints(
                {.margins = {0, 0, metrics::trailingGlyphMargin(disclosure->iconName(), disclosure->iconSize(), 5), 0}});
        row->setGeometry(localRect());
    }
};

Unfold::Unfold()
{
    m_content = addChild<ColumnLayout>();
    setClipsChildren(true);
    setLayoutHints({.fillWidth = true});
}

Unfold::~Unfold() = default;

void Unfold::setShown(bool shown)
{
    if (m_shown == shown)
        return;
    m_shown = shown;
    Window* window = this->window();
    if (!window) {
        m_progress = 1;
        requestLayout();
        return;
    }
    if (!m_animation) {
        m_animation = std::make_unique<Animation>(window);
        m_animation->setDuration(std::chrono::milliseconds(200));
        m_animation->setEasing(Animation::Easing::OutSine);
        m_animation->onUpdate = [this](double value) {
            m_progress = value;
            requestLayout();
        };
    }
    // From wherever it is now, as a Behavior retargets.
    m_from = m_height;
    m_progress = 0;
    m_animation->start();
    requestLayout();
}

double Unfold::targetHeight(double width) const
{
    return m_shown ? m_content->heightForWidth(width) : 0;
}

SizeF Unfold::sizeHint() const
{
    return {m_content->sizeHint().width, heightForWidth(geometry().width)};
}

double Unfold::heightForWidth(double width) const
{
    return m_from + (targetHeight(width) - m_from) * m_progress;
}

void Unfold::layout()
{
    const double width = geometry().width;
    m_height = heightForWidth(width);
    if (m_progress >= 1)
        m_from = m_height;
    // Filling the unfolding height, as the anchored layout did: it squeezes
    // its rows while short.
    m_content->setGeometry({0, 0, width, m_height});
    m_content->setVisible(m_height > 0);
}

UserCard::UserCard(std::shared_ptr<accounts::UserAccount> account, int64_t loggedInUid)
    : Card(Card::Style::Padded)
    , m_account(std::move(account))
    , m_loggedInUid(loggedInUid)
{
    ColumnLayout* main = addChild<ColumnLayout>();
    main->setSpacing(0);
    RowLayout* itemRow = main->addChild<RowLayout>();
    itemRow->setSpacing(0);
    m_header = itemRow->addChild<Header>();
    m_header->setLayoutHints({.fillWidth = true});
    m_unfold = main->addChild<Unfold>();
    m_unfold->content()->setSpacing(metrics::cardSpacing);

    GridLayout* grid = m_unfold->content()->addChild<GridLayout>(2);
    grid->setColumnSpacing(metrics::formColumnSpacing);
    grid->setRowSpacing(metrics::formRowSpacing);
    grid->setLayoutHints({.fillWidth = true, .margins = {0, metrics::cardSpacing, 0, 0}});
    grid->addChild<Label>("Account type");
    m_accountType = grid->addChild<Label>();

    m_password = m_unfold->content()->addChild<StandardButton>("Change password");
    m_password->onClicked = [this] {
        if (onChangePassword)
            onChangePassword();
    };
    m_delete = m_unfold->content()->addChild<StandardButton>("Delete this user");
    m_delete->onClicked = [this] {
        if (onDelete)
            onDelete();
    };

    m_header->avatar->onClicked = [this] {
        if (onPictureClicked)
            onPictureClicked();
    };
    m_header->disclosure->onClicked = [this] {
        m_unfold->toggle();
        update();
    };
    m_header->onDoubleClicked = [this] {
        m_unfold->toggle();
        update();
    };

    m_changed = m_account->changed.connect([this] { update(); });
    update();
}

UserCard::~UserCard() = default;

void UserCard::layout()
{
    const Color background = color(SettingsColor::UserCardButtonBackground);
    m_password->setBackgroundColor(background);
    m_delete->setBackgroundColor(background);
    Card::layout();
}

void UserCard::showPicture(const std::string& path)
{
    m_header->avatar->setPicture(path);
}

void UserCard::update()
{
    const std::string userName = m_account->userName();
    const std::string realName = m_account->realName();
    m_header->avatar->setUserName(userName);
    m_header->avatar->setPicture(m_account->iconFileName());
    m_header->userName->setText(userName);
    m_header->realName->setText(realName);
    m_header->realName->setVisible(realName != userName);
    m_header->loggedIn->setVisible(m_account->userId() == m_loggedInUid);
    m_header->disclosure->setIconName(m_unfold->isShown() ? "chevron-up" : "chevron-down");
    m_accountType->setText(m_account->accountType() == accounts::UserAccount::Standard ? "Standard"
                                                                                        : "Administrator");
    m_delete->setEnabled(m_account->userId() != m_loggedInUid);
    requestLayout();
}

} // namespace settings
