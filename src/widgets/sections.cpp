// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/sections.h"

#include "ui/color_mixer.h"
#include "widgets/metrics.h"

using namespace cutefish::ui;

namespace settings {

SectionLabel::SectionLabel(std::string text)
{
    setMargins({metrics::cardHorizontalPadding, 0, metrics::cardHorizontalPadding, 0});
    setLayoutHints({.fillWidth = true});
    m_label = addChild<Label>(std::move(text));
    m_label->setWeight(500);
    m_label->setColor(withAlpha(ColorId::Text, 0.85));
    m_label->setLayoutHints({.fillWidth = true});
}

Footnote::Footnote(std::string text)
    : SectionLabel(std::move(text))
{
    label()->setWrap(true);
    label()->setWeight(400);
    label()->setColor(ColorId::DisabledText);
}

void Footnote::setColor(std::optional<ColorRecipe> color)
{
    label()->setColor(std::move(color).value_or(ColorId::DisabledText));
}

Section::Section(std::string title)
{
    setSpacing(metrics::sectionLabelSpacing);
    m_title = addChild<SectionLabel>(std::move(title));
}

} // namespace settings
