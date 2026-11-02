// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/emptylistoverlay.h"

#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"

using namespace cutefish::ui;

namespace settings {

void StackView::layout()
{
    for (const auto& child : children())
        child->setGeometry(localRect());
}

EmptyListOverlay::EmptyListOverlay()
{
    m_label = addChild<Label>();
}

void EmptyListOverlay::layout()
{
    m_label->setColor(Theme::current().disabledText);
    const SizeF size = m_label->sizeHint();
    m_label->setGeometry({rounding::centeredStart(geometry().width, size.width),
                          rounding::centeredStart(geometry().height, size.height), size.width, size.height});
}

} // namespace settings
