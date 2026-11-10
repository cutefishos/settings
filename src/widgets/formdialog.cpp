// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/formdialog.h"

#include <algorithm>

#include "base/event_loop.h"
#include "ui/keys.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "ui/window_controls.h"
#include "ui/window_header.h"
#include "widgets/metrics.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;

namespace settings {

static constexpr double kHeaderHeight = 44;
static constexpr double kTitlePixelSize = 14;
// With the window controls, a title bar as the toolkit's dialogs have: 32
// high, the title centred on the window in a smaller size, clear of the
// controls in their 32-pixel slot.
static constexpr double kBarHeight = 32;
static constexpr double kBarTitlePixelSize = 12;
static constexpr double kBarTitleInset = metrics::dialogWindowButtonsMargin + window_controls::kRowWidth
        + (metrics::xxl - window_controls::kButton) + metrics::s;

// A ColumnLayout anchored to the window's left, right and top with the
// dialog's padding, the title first in it; with the window controls, the
// title moves into a title bar over the column instead. The window is as
// tall as the layout and the padding.
class FormRoot : public View
{
public:
    explicit FormRoot(std::string text)
    {
        column = addChild<ColumnLayout>();
        column->setSpacing(metrics::dialogSpacing);
        title = column->addChild<Label>(text);
        title->setFontSize(kTitlePixelSize);
        title->setWeight(600);
        title->setWrap(true);
        title->setLayoutHints({.fillWidth = true});
        header = addChild<WindowHeader>(kBarHeight);
        header->setMinimizeEnabled(false);
        header->setMaximizeEnabled(false);
        header->setButtonMargin(metrics::dialogWindowButtonsMargin);
        barTitle = addChild<Label>(std::move(text));
        barTitle->setFontSize(kBarTitlePixelSize);
        barTitle->setTextAlignment(Label::TextAlignment::Center);
        barTitle->setVerticallyCentered(true);
        setControlsVisible(true);
    }

    void setControlsVisible(bool visible)
    {
        controls = visible;
        header->setCloseVisible(visible);
        header->setMinimizeVisible(visible);
        header->setMaximizeVisible(visible);
        barTitle->setVisible(visible);
        title->setVisible(!visible);
        requestLayout();
    }

    SizeF sizeHint() const override
    {
        const SizeF size = column->sizeHint();
        return {size.width + 2 * metrics::dialogPadding, size.height + top() + metrics::dialogPadding};
    }

    double heightForWidth(double width) const override
    {
        return column->heightForWidth(std::max(0.0, width - 2 * metrics::dialogPadding)) + top() + metrics::dialogPadding;
    }

    ColumnLayout* column;
    Label* title;
    Label* barTitle;
    WindowHeader* header;
    bool controls = true;

protected:
    void layout() override
    {
        // Not filled to the window: a fixed-height dialog gives the rest to the
        // layout's filling items, a fitted one is exactly as tall.
        const double width = std::max(0.0, geometry().width - 2 * metrics::dialogPadding);
        column->setGeometry({metrics::dialogPadding, top(), width,
                             std::max(0.0, geometry().height - top() - metrics::dialogPadding)});
        header->setGeometry({0, 0, geometry().width, controls ? kBarHeight : kHeaderHeight});
        barTitle->setGeometry({kBarTitleInset, 0, std::max(0.0, geometry().width - 2 * kBarTitleInset), kBarHeight});
    }

private:
    double top() const { return controls ? kBarHeight : metrics::dialogPadding; }
};

FormDialog::FormDialog(SettingsWindow& parent, std::string title, double width)
    : m_window(std::make_unique<Window>())
    , m_width(width)
{
    auto root = std::make_unique<FormRoot>(title);
    m_root = root.get();
    m_window->setTitle(title);
    m_window->setContentView(std::move(root));
    m_window->setTransientParent(&parent.window(), true);
    m_window->onCloseRequested = [this] { requestClose(); };
    m_window->addShortcut(NoModifier, key::Escape, [this] { requestClose(); });
}

FormDialog::~FormDialog() = default;

ColumnLayout* FormDialog::content() const
{
    return m_root->column;
}

Label* FormDialog::titleLabel() const
{
    return m_root->title;
}

void FormDialog::setTitle(std::string title)
{
    m_root->title->setText(title);
    m_root->barTitle->setText(title);
    if (m_window)
        m_window->setTitle(title);
}

RowLayout* FormDialog::addButtonRow()
{
    RowLayout* row = m_root->column->addChild<RowLayout>();
    row->setSpacing(metrics::dialogButtonSpacing);
    row->setLayoutHints({.horizontalAlignment = Alignment::End});
    return row;
}

void FormDialog::setFixedHeight(double height)
{
    m_fixedHeight = height;
}

void FormDialog::setCloseVisible(bool visible)
{
    m_root->setControlsVisible(visible);
}

void FormDialog::setBackgroundColor(std::optional<ColorRecipe> color)
{
    m_background = std::move(color);
    if (m_window)
        m_window->setBackgroundColor(m_background.value_or(ColorId::SecondBackground));
}

void FormDialog::show()
{
    if (!m_window)
        return;
    m_window->setBackgroundColor(m_background.value_or(ColorId::SecondBackground));
    if (m_fixedHeight > 0) {
        m_window->resize({m_width, m_fixedHeight});
        m_window->setMinimumSize({m_width, m_fixedHeight});
        m_window->setMaximumSize({m_width, m_fixedHeight});
    } else {
        m_window->resize({m_width, 0});
        m_window->setFitsContentHeight(true);
    }
    m_window->show();
    if (onShown)
        onShown();
}

void FormDialog::requestClose()
{
    if (canClose && !canClose())
        return;
    close();
}

void FormDialog::close()
{
    if (!m_window)
        return;
    // The window may be handling the event that closes it.
    std::shared_ptr<Window> window(std::move(m_window));
    cutefish::base::EventLoop::current()->post([window] {});
    if (onClosed)
        onClosed();
}

} // namespace settings
