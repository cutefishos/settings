// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/sections.h"

#include "ui/theme.h"
#include "widgets/metrics.h"

using namespace cutefish::ui;

namespace settings {

SectionLabel::SectionLabel(std::string text, bool wrap)
{
    setMargins({metrics::cardHorizontalPadding, 0, metrics::cardHorizontalPadding, 0});
    setLayoutHints({.fillWidth = true});
    m_label = addChild<Label>(std::move(text));
    m_label->setWrap(wrap);
    m_label->setColor(Theme::current().disabledText);
    m_label->setLayoutHints({.fillWidth = true});
}

void SectionLabel::setColor(std::optional<Color> color)
{
    m_color = color;
    m_label->setColor(m_color.value_or(Theme::current().disabledText));
}

void SectionLabel::layout()
{
    m_label->setColor(m_color.value_or(Theme::current().disabledText));
    Column::layout();
}

Section::Section(std::string title)
{
    setSpacing(metrics::sectionLabelSpacing);
    m_title = addChild<SectionLabel>(std::move(title));
}

} // namespace settings
