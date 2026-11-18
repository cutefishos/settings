// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/formdialog.h"

#include "ui/cell_layout.h"
#include "ui/dialog_window.h"
#include "ui/keys.h"
#include "ui/window.h"
#include "widgets/metrics.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;

namespace settings {

FormDialog::FormDialog(SettingsWindow& parent, std::string title, double width)
    : m_dialog(std::make_unique<DialogWindow>(&parent.window(), std::move(title), width))
{
    m_content = m_dialog->setContent<ColumnLayout>();
    m_content->setSpacing(metrics::dialogSpacing);
    m_dialog->onClosed = [this] {
        if (onClosed)
            onClosed();
    };
    m_dialog->window()->onCloseRequested = [this] { requestClose(); };
    m_dialog->window()->addShortcut(NoModifier, key::Escape, [this] { requestClose(); });
}

FormDialog::~FormDialog() = default;

Window* FormDialog::window() const
{
    return m_dialog->window();
}

void FormDialog::setTitle(std::string title)
{
    // The title's label goes with the window.
    if (isOpen())
        m_dialog->setTitle(std::move(title));
}

RowLayout* FormDialog::addButtonRow()
{
    RowLayout* row = m_content->addChild<RowLayout>();
    row->setSpacing(metrics::dialogButtonSpacing);
    row->setLayoutHints({.horizontalAlignment = Alignment::End});
    return row;
}

void FormDialog::setFixedHeight(double height)
{
    m_dialog->setHeight(height);
}

void FormDialog::show()
{
    if (!isOpen())
        return;
    m_dialog->show();
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
    if (!isOpen())
        return;
    m_dialog->close();
}

} // namespace settings
