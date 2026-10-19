// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <optional>
#include <string>

#include "ui/view.h"

namespace cutefish::ui {
class Label;
}

namespace settings {

// A full-width button standing alone between cards: a flat plate that only
// darkens while pressed.
class StandardButton : public cutefish::ui::View
{
public:
    explicit StandardButton(std::string text = {});

    void setText(std::string text);
    // Unset: #363636 in dark mode, white otherwise.
    void setBackgroundColor(std::optional<cutefish::ui::Color> color);

    cutefish::ui::SizeF sizeHint() const override;
    bool acceptsPointer() const override { return true; }
    bool acceptsFocus() const override { return true; }

    std::function<void()> onClicked;

protected:
    void layout() override;
    void paint(cutefish::ui::Painter& painter) override;
    void pointerPress(const cutefish::ui::PointerEvent& event) override;
    void pointerRelease(const cutefish::ui::PointerEvent& event) override;
    bool keyPress(const cutefish::ui::KeyEvent& event) override;

private:
    cutefish::ui::Color backgroundColor() const;

    cutefish::ui::Label* m_label;
    std::optional<cutefish::ui::Color> m_background;
    bool m_pressed = false;
};

} // namespace settings
