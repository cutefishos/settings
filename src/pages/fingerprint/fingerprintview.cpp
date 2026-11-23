// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/fingerprint/fingerprintview.h"

#include <algorithm>
#include <chrono>

#include "pages/fingerprint/fingerprintglyph.h"
#include "ui/animation.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"

using namespace cutefish::ui;

namespace settings {

// How far the colour has to reach from the middle, in sides of the glyph, to
// cover its lower corners.
static constexpr double kReach = 0.6;

FingerprintView::FingerprintView() = default;

FingerprintView::~FingerprintView() = default;

void FingerprintView::build()
{
    const SizeF size = geometry().size();
    m_builtFor = size;
    const double side = std::min(size.width, size.height);
    m_glyph = fingerprintGlyph(side).translated((size.width - side) / 2, (size.height - side) / 2);
}

void FingerprintView::layout()
{
    if (geometry().size() != m_builtFor)
        build();
}

void FingerprintView::setProgress(double progress)
{
    progress = std::clamp(progress, 0.0, 1.0);
    if (progress == m_target)
        return;
    m_from = m_shown;
    m_target = progress;
    if (!window()) {
        m_shown = m_target;
        requestPaint();
        return;
    }
    if (!m_animation) {
        m_animation = std::make_unique<Animation>(window());
        m_animation->setDuration(std::chrono::milliseconds(220));
        m_animation->setEasing(Animation::Easing::OutCubic);
        m_animation->onUpdate = [this](double value) {
            m_shown = m_from + (m_target - m_from) * value;
            requestPaint();
        };
    }
    m_animation->start();
}

void FingerprintView::setComplete(bool complete)
{
    if (complete == m_complete)
        return;
    m_complete = complete;
    requestPaint();
}

void FingerprintView::paint(Painter& painter)
{
    if (m_glyph.isEmpty())
        build();
    const Theme& theme = Theme::current();
    const Color lit = m_complete ? accent::green : theme.highlight;
    painter.fillPath(m_glyph, theme.text.withAlpha(theme.darkMode ? 0.22 : 0.16));
    if (m_shown <= 0 && !m_complete)
        return;
    if (m_shown >= 1 || m_complete) {
        painter.fillPath(m_glyph, lit);
        return;
    }
    const SizeF size = geometry().size();
    const double side = std::min(size.width, size.height);
    const double radius = side * kReach * m_shown;
    Path reached;
    reached.addEllipse({size.width / 2 - radius, size.height / 2 - radius, 2 * radius, 2 * radius});
    painter.fillPath(m_glyph.intersected(reached), lit);
}

} // namespace settings
