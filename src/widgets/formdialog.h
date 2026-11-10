// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>

#include "ui/color_mixer.h"
#include "ui/geometry.h"

namespace cutefish::ui {
class ColumnLayout;
class Label;
class RowLayout;
class Window;
}

namespace settings {

class FormRoot;
class SettingsWindow;

// Settings' own form dialogs (add a user, change a password, add a language,
// join a network, edit a shortcut): a modal window over the settings window
// with the window controls and its title centred in a title bar, or, without
// them, its title in semibold first in the content; and a column of content
// below. Its height follows the content unless fixed.
// A dialog is shown once: after it closes, make a new one.
class FormDialog
{
public:
    FormDialog(SettingsWindow& parent, std::string title, double width);
    virtual ~FormDialog();

    FormDialog(const FormDialog&) = delete;
    FormDialog& operator=(const FormDialog&) = delete;

    cutefish::ui::Window* window() const { return m_window.get(); }
    // The dialog's ColumnLayout, inside its padding: the title first (hidden
    // while the title bar holds it), then what goes under it, with the
    // dialog's spacing.
    cutefish::ui::ColumnLayout* content() const;
    cutefish::ui::Label* titleLabel() const;
    void setTitle(std::string title);
    // A RowLayout of buttons aligned to the right, as the last item of the
    // content.
    cutefish::ui::RowLayout* addButtonRow();

    // A fixed height lets the content's filling items share the space.
    void setFixedHeight(double height);
    // The window controls; hidden for dialogs whose own Cancel or Close button
    // closes them, the title then first in the content.
    void setCloseVisible(bool visible);
    // Unset: the theme's second background colour.
    void setBackgroundColor(std::optional<cutefish::ui::ColorRecipe> color);

    void show();
    // Asks canClose first, as the close button and Escape do.
    void requestClose();
    // Closes at once; the window goes once the event being handled is over.
    void close();
    bool isOpen() const { return m_window != nullptr; }

    // Returns false to keep the dialog open, e.g. while waiting for a reply.
    std::function<bool()> canClose;
    std::function<void()> onShown;
    std::function<void()> onClosed;

private:
    std::unique_ptr<cutefish::ui::Window> m_window;
    FormRoot* m_root;
    double m_width;
    double m_fixedHeight = 0;
    std::optional<cutefish::ui::ColorRecipe> m_background;
};

} // namespace settings
