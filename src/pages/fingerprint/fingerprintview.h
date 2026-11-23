// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>

#include "ui/path.h"
#include "ui/view.h"

namespace cutefish::ui {
class Animation;
}

namespace settings {

// A print of dashed ridges. It starts grey and takes the
// accent colour from the middle outwards as the print is recorded; a
// complete print turns green.
class FingerprintView : public cutefish::ui::View
{
public:
    FingerprintView();
    ~FingerprintView() override;

    // How much of the print is recorded, from 0 to 1; the colour follows it
    // over a moment.
    void setProgress(double progress);
    double progress() const { return m_target; }
    // How far the colour has reached now, which lags progress().
    double shownProgress() const { return m_shown; }
    void setComplete(bool complete);

    cutefish::ui::SizeF sizeHint() const override { return {kSize, kSize}; }

    static constexpr double kSize = 160;

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;

private:
    void build();

    cutefish::ui::Path m_glyph;
    cutefish::ui::SizeF m_builtFor;
    std::unique_ptr<cutefish::ui::Animation> m_animation;
    double m_target = 0;
    double m_shown = 0;
    double m_from = 0;
    bool m_complete = false;
};

} // namespace settings
