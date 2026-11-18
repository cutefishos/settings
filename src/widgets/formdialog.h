// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <memory>
#include <string>

namespace cutefish::ui {
class ColumnLayout;
class DialogWindow;
class RowLayout;
class Window;
}

namespace settings {

class SettingsWindow;

// Settings' own form dialogs (add a user, change a password, add a language,
// join a network, edit a shortcut): the shared dialog window over the
// settings window, closed by its close control or Escape, and a column of
// content below the title. Its height follows the content unless fixed.
// A dialog is shown once: after it closes, make a new one.
class FormDialog
{
public:
    FormDialog(SettingsWindow& parent, std::string title, double width);
    virtual ~FormDialog();

    FormDialog(const FormDialog&) = delete;
    FormDialog& operator=(const FormDialog&) = delete;

    // Null once the dialog has closed.
    cutefish::ui::Window* window() const;
    // The dialog's ColumnLayout, inside its padding, with the dialog's
    // spacing.
    cutefish::ui::ColumnLayout* content() const { return m_content; }
    void setTitle(std::string title);
    // A RowLayout of buttons aligned to the right, as the last item of the
    // content.
    cutefish::ui::RowLayout* addButtonRow();

    // The whole window's height; lets the content's filling items share the
    // space.
    void setFixedHeight(double height);

    void show();
    // Asks canClose first, as the close control and Escape do.
    void requestClose();
    // Closes at once; the window goes once the event being handled is over.
    void close();
    bool isOpen() const { return window() != nullptr; }

    // Returns false to keep the dialog open, e.g. while waiting for a reply.
    std::function<bool()> canClose;
    std::function<void()> onShown;
    std::function<void()> onClosed;

private:
    std::unique_ptr<cutefish::ui::DialogWindow> m_dialog;
    cutefish::ui::ColumnLayout* m_content;
};

} // namespace settings
